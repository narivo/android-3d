package com.example.webviewar;

import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public final class AssetsUtils
{
    private static final String TAG= AssetsUtils.class.getSimpleName();

    static ArrayList<String> folderHierarchy = new ArrayList<>();

    public static ArrayList<String> getFolderHierarchy(AssetManager assets) {
        folderHierarchy = new ArrayList<>();
        listAssetsFolders(assets, "");
        return folderHierarchy;
    }

    public static void listAssetsFolders(AssetManager assets, String path) {
        try {
            String[] allFiles = assets.list(path);
            for (String file : allFiles) {
                // direct childs
                String subdir = path.isEmpty()? file : path + "/" + file;
                if(isDirectory(assets, subdir)) {
                    folderHierarchy.add(subdir);
                    // sub-folders
                    listAssetsFolders(assets, subdir);
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "an error occured, cannot opent assets folder");
        }
    }

    public static boolean isDirectory(AssetManager assets, String name) {
        try {
            String[] allFiles = assets.list(name);
            return allFiles.length > 0;
        }catch (IOException e) {
            Log.e(TAG, "an error occured, cannot opent assets folder " + name);
            return false;
        }
    }
}
