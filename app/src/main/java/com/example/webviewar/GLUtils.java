package com.example.webviewar;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.IntBuffer;
import java.nio.charset.StandardCharsets;

import static android.opengl.GLES30.*;

public final class GLUtils  {

    private static final String TAG = GLUtils.class.getSimpleName();
    public static AssetManager assets;

    public static int createProgram(String vertexPath, String fragmentPath) {
        try {
            InputStream vertShInStm = assets.open(vertexPath);
            InputStream fragShInStm = assets.open(fragmentPath);

            String vertexCode = readStream(vertShInStm);
            String fragmentCode = readStream(fragShInStm);

            int vertex = createCompileShader(GL_VERTEX_SHADER, vertexCode);
            int fragment = createCompileShader(GL_FRAGMENT_SHADER, fragmentCode);

            int ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);

            glLinkProgram(ID);
            checkCompileErrors(ID, "program");

            glDeleteShader(vertex);
            glDeleteShader(fragment);

            return ID;
        } catch (IOException e) {
            e.printStackTrace(); return -1;
        }
    }

    private static String readStream(InputStream input) {
        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();

            // read data from input stream
            byte[] buffer = new byte[1024];
            int length;
            while ((length = input.read(buffer)) != -1) {
                baos.write(buffer, 0, length);
            }

            return baos.toString(StandardCharsets.UTF_8.name());
        } catch (IOException ex) {
            ex.printStackTrace();
            return "";
        }
    }

    private static int createCompileShader(int type, String shaderCode) {
        if(type != GL_FRAGMENT_SHADER && type != GL_VERTEX_SHADER) {
            throw new IllegalArgumentException("Shader type must be one of: GL_FRAGMENT_SHADER or GL_VERTEX_SHADER constant");
        }
        int shader = glCreateShader(type);

        glShaderSource(shader, shaderCode);
        glCompileShader(shader);
        checkCompileErrors(shader, "shader");

        return shader;

    }

    private static void checkCompileErrors(int shader, String type) {
        IntBuffer success = IntBuffer.allocate(1);
        if(type.toLowerCase().contains("program")) {
            glGetProgramiv(shader, GL_LINK_STATUS, success);
            if(success.get(0) == 0) {
                String infoLog = glGetProgramInfoLog(shader);
                Log.e(TAG,"ERROR::PROGRAM::LINKING_FAILED\n Reason: \n " + infoLog);
            }
        } else {
            glGetShaderiv(shader, GL_COMPILE_STATUS, success);
            if(success.get(0) == 0) {
                String infoLog = glGetShaderInfoLog(shader);
                Log.e(TAG, "ERROR::SHADER::COMPILATION_FAILED\n Reason: \n " + infoLog);
            }
        }
    }

    public static int loadTexture(String path, int activeTex) {
        int textureID = -1;
        try(InputStream inStm = assets.open(path)) {
            BitmapFactory.Options op = new BitmapFactory.Options();
            op.inPreferredConfig = Bitmap.Config.ARGB_8888;
            Bitmap bmp = BitmapFactory.decodeStream(inStm, null, op);

            // generate textureID
            IntBuffer textures = IntBuffer.allocate(1);
            glGenTextures(1, textures);
            glActiveTexture(GL_TEXTURE0+activeTex);
            textureID = textures.get(0);

            // create texture
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            android.opengl.GLUtils.texImage2D(GL_TEXTURE_2D, 0, bmp, 0);
            if(bmp != null) bmp.recycle();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return textureID;
    }
}
