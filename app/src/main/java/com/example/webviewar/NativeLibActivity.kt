package com.example.webviewar

import android.Manifest
import android.app.ActivityManager
import android.content.pm.PackageManager
import android.content.res.AssetManager
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import kotlinx.android.synthetic.main.activity_native_lib.*
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class NativeLibActivity: AppCompatActivity() {

    companion object {
        private const val WRITE_PERMISSION_CODE = 251
        private val TAG = NativeLibActivity::class.java.simpleName
        var permissionGranted = false
        init {
            System.loadLibrary("gles3jni")
        }
    }

    external fun load(mgr: AssetManager)

    external fun getHello(): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_native_lib)
        // Check if the system supports OpenGL ES 2.0.
        val activityManager = getSystemService(ACTIVITY_SERVICE) as ActivityManager
        val supportsEs3 = activityManager.deviceConfigurationInfo.reqGlEsVersion >= 0x30000

        if (supportsEs3) {
            // Request an OpenGL ES 3.0 compatible context.
            surfacevw.setEGLContextClientVersion(3)

            // Set the native renderer to our demo renderer,defined below.
            surfacevw.setRenderer(Renderer())
        } else {
            // This is where you could create an OpenGL ES 1.x compatible
            // renderer if you wanted to support both ES 1 and ES 2.
                Log.e(TAG, "Do not support ES3")
            return
        }

        if(ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
            != PackageManager.PERMISSION_GRANTED) {
            // Permission is not granted
            ActivityCompat.requestPermissions(this,
                arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE), WRITE_PERMISSION_CODE)
        } else {
            permissionGranted = true
        }
    }

    override fun onResume() {
        super.onResume()
        surfacevw.onResume()
        if(permissionGranted) {
            load(resources.assets)
        }
    }

    override fun onPause() {
        super.onPause()
        surfacevw.onPause()
    }

    // This function is called when the user accepts or decline the permission.
    // Request Code is used to check which permission called this function.
    // This request code is provided when the user is prompt for permission.
    override fun onRequestPermissionsResult(requestCode: Int,
                                            permissions: Array<String>,
                                            grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == WRITE_PERMISSION_CODE) {
            permissionGranted = if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(this, "Write Permission Granted", Toast.LENGTH_SHORT).show()
                true
            } else {
                Toast.makeText(this, "Write Permission Denied", Toast.LENGTH_SHORT).show()
                false
            }
        }
    }
}

class Renderer: GLSurfaceView.Renderer {

    companion object {
        init {
            System.loadLibrary("gles3jni")
        }
    }

    external fun nativeSurfaceChanged(width: Int, height: Int)

    external fun nativeSurfaceCreated()

    external fun nativeDrawFrame()

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        nativeSurfaceCreated()
    }

    override fun onSurfaceChanged(p0: GL10?, p1: Int, p2: Int) {
        nativeSurfaceChanged(p1, p2)
    }

    override fun onDrawFrame(p0: GL10?) {
        nativeDrawFrame()
    }

}