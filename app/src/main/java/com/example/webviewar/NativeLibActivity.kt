package com.example.webviewar

import android.Manifest
import android.app.ActivityManager
import android.content.Context
import android.content.pm.PackageManager
import android.content.res.AssetManager
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.util.Log
import android.view.GestureDetector
import android.view.MotionEvent
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.view.GestureDetectorCompat
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

    lateinit var surfacevw: GLView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        surfacevw = GLView(this)
        setContentView(surfacevw)
        // Check if the system supports OpenGL ES 3.0.

        globalAssets = assets
        val activityManager = getSystemService(ACTIVITY_SERVICE) as ActivityManager
        val supportsEs3 = activityManager.deviceConfigurationInfo.reqGlEsVersion >= 0x30000

        if (supportsEs3) {
            // Request an OpenGL ES 3.0 compatible context.
            surfacevw.setEGLContextClientVersion(3)

            // Set the native renderer to our demo renderer,defined below.
            surfacevw.setRenderer(Renderer(assets))
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
            load(assets)
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

var globalAssets: AssetManager? = null

/*class Renderer: GLSurfaceView.Renderer {

    companion object {
        private val TAG = Renderer::class.java.simpleName
        init {
            System.loadLibrary("gles3jni")
        }
    }

    @JvmField val sharedShader = Shader
    @JvmField val sharedTexture = Texture

    external fun nativeSurfaceChanged(width: Int, height: Int)

    external fun nativeSurfaceCreated()

    external fun nativeDrawFrame()

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        sharedTexture.assets = globalAssets
        nativeSurfaceCreated()
    }

    override fun onSurfaceChanged(p0: GL10?, p1: Int, p2: Int) {
        nativeSurfaceChanged(p1, p2)
    }

    override fun onDrawFrame(p0: GL10?) {
        nativeDrawFrame()
    }

}*/

class GLView(ctx: Context) : GLSurfaceView(ctx), GestureDetector.OnGestureListener {

    companion object {
        private val TAG = GLView::class.java.simpleName
        init {
            System.loadLibrary("gles3jni")
        }
    }

    external fun nativeCameraRotation(xoffest: Float, yoffset: Float);

    var gestureDetector: GestureDetectorCompat = GestureDetectorCompat(ctx, this)

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        return gestureDetector.onTouchEvent(event)
    }

    override fun onDown(event: MotionEvent): Boolean {
        Log.d(TAG, "onDown: $event")
        return true
    }

    override fun onShowPress(event: MotionEvent?) {
        Log.d(TAG, "onShowPress: $event")
    }

    override fun onSingleTapUp(event: MotionEvent?): Boolean {
        Log.d(TAG, "onSingleTapUp: $event")
        return false
    }

    override fun onScroll(e1: MotionEvent?, e2: MotionEvent?, distanceX: Float, distanceY: Float): Boolean {
        Log.d(TAG, "onScroll: $e1 $e2")
        // all width = 89 deg
        // all height = 179 deg
        var yaw = (distanceX*179)/width
        var pitch = (distanceY*89)/height
        // left->right = neg | right->left = pos
        // top-> bottom = neg| bottom->top = pos
        nativeCameraRotation(distanceX, distanceY)
        return true
    }

    override fun onLongPress(event: MotionEvent?) {
        Log.d(TAG, "onLongPress: $event")
    }

    override fun onFling(e1: MotionEvent?, e2: MotionEvent?, velocityX: Float, velocityY: Float): Boolean {
        Log.d(TAG, "onFling: $e1 $e2")
        return true
    }

}