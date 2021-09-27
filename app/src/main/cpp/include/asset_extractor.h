//
// Created by MadiApps on 24/09/2021.
//

#ifndef ASSET_EXTRACTOR_H
#define ASSET_EXTRACTOR_H

#include "jni.h"

#include <android/asset_manager_jni.h>

#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string>

#include "logger.h"

extern int errno;

using namespace std;

class AssetExtractor{
public:

    void ExtractToCache() {
        folders = getAssetsHierarchy();

        jclass activityClass = env->GetObjectClass(thiz);
        jmethodID getCacheDir = env->GetMethodID(activityClass, "getExternalCacheDir", "()Ljava/io/File;");
        jobject file = env->CallObjectMethod(thiz, getCacheDir);

        jclass fileClass = env->FindClass("java/io/File");
        jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
        jstring jpath = (jstring)env->CallObjectMethod(file, getAbsolutePath);
        const char* app_dir = env->GetStringUTFChars(jpath, NULL);

        // chdir in the application cache directory
        LOGI("app_dir: %s", app_dir);
        chdir(app_dir);

        AAssetDir* assetDir = AAssetManager_openDir(aAssetManager, "");
        extractDir(assetDir, "");
        AAssetDir_close(assetDir);

        for(int i = 0; i < folders.size(); i++) {
            extractFolder(app_dir, folders[i].c_str());
        }

        env->ReleaseStringUTFChars(jpath, app_dir);
    }

    AssetExtractor(JNIEnv* penv, jobject pthiz, jobject pmgr) {
        aAssetManager = AAssetManager_fromJava(penv, pmgr);
        if (aAssetManager == NULL) {
            LOGE("error loading asset manager");
        } else {
            LOGI( "loaded asset  manager");
        }
        env = penv;
        thiz = pthiz;
        mgr = pmgr;
    }

    ~AssetExtractor() {
        env->DeleteLocalRef(mgr);
    }

private:
    JNIEnv* env;
    jobject thiz;
    jobject mgr;
    AAssetManager* aAssetManager;
    vector<string> folders;
    struct stat st = {0};

    void extractFolder(const char* root_dir, const char* name) {
        if(stat(name, &st) == -1) {
            mkdir(name, 0700);
        }

        string obj_dir = string(root_dir) + "/" + name;
        chdir(obj_dir.c_str());

        AAssetDir* backpackDir = AAssetManager_openDir(aAssetManager, name);
        extractDir(backpackDir, name);
        AAssetDir_close(backpackDir);

        chdir(root_dir);
    }

    void extractDir(AAssetDir *assetDir, string dirname) {
        const char* filename = (const char*)NULL;
        while ((filename = AAssetDir_getNextFileName(assetDir)) != NULL) {
            if(dirname.empty())
                extractFiles(filename, filename);
            else {
                string fqdname = dirname+"/"+string(filename);
                extractFiles(fqdname.c_str(), filename);
            }
        }
    }

    void extractFiles(const char *filename, const char* rawfilename) {
        AAsset* asset = AAssetManager_open(aAssetManager, filename, AASSET_MODE_STREAMING);
        char buf[BUFSIZ];
        int nb_read = 0;
        FILE* out = fopen(rawfilename, "w+");
        LOGI("error :  %s", strerror(errno));
        while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
            fwrite(buf, nb_read, 1, out);
        fclose(out);
        AAsset_close(asset);
    }

    vector<string> getAssetsHierarchy() {
        vector<string> folders;

        jclass assetsUtilsKlass = env->FindClass("com/example/webviewar/AssetsUtils");
        jmethodID getFolderHierarchy = env->GetStaticMethodID(assetsUtilsKlass, "getFolderHierarchy",
                                                              "(Landroid/content/res/AssetManager;)Ljava/util/ArrayList;");
        jobject foldersList = env->CallStaticObjectMethod(assetsUtilsKlass, getFolderHierarchy, mgr);
        jclass listKlass = env->FindClass("java/util/ArrayList");
        jmethodID size = env->GetMethodID(listKlass, "size", "()I");

        int length = env->CallIntMethod(foldersList, size);
        for(int i = 0; i < length; i++) {
            jmethodID get = env->GetMethodID(listKlass, "get", "(I)Ljava/lang/Object;");
            jstring eleM = (jstring) env->CallObjectMethod(foldersList, get, i);
            string elem = env->GetStringUTFChars(eleM, NULL);
            folders.push_back(elem);
        }

        return folders;
    }
};

#endif //ASSET_EXTRACTOR_H
