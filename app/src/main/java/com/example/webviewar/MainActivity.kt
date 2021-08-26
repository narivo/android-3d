package com.example.webviewar

import android.annotation.SuppressLint
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.webkit.*
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import androidx.webkit.WebViewAssetLoader


class MainActivity : AppCompatActivity() {

    lateinit var webvw: WebView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        webvw = findViewById(R.id.webview)
        webvw.settings.javaScriptEnabled = true
        webvw.settings.domStorageEnabled = true
        webvw.webChromeClient = ChromeClient()
        webvw.addJavascriptInterface(JSInterface(this), "AndroidFunctions")

        // domain = appassets.androidplatform.net
        // url = domain + /assets/
        val assetLoader: WebViewAssetLoader = WebViewAssetLoader.Builder()
            .addPathHandler("/assets/", WebViewAssetLoader.AssetsPathHandler(this))
            .build()

        webvw.webViewClient = object : WebViewClient() {
            @RequiresApi(21)
            override fun shouldInterceptRequest(
                view: WebView,
                request: WebResourceRequest
            ): WebResourceResponse? {
                return assetLoader.shouldInterceptRequest(request.url)
            }
        }

        webvw.loadUrl("https://appassets.androidplatform.net/assets/custom-patterns.html")
//        webvw.loadUrl("https://fcor.github.io/arjs-gestures/index.html")
    }

    override fun onResume() {
        super.onResume()
        checkCameraPermission()
    }

    private fun checkCameraPermission() {
        if (CameraPermission.hasCameraPermission(this) == false) {
            CameraPermission.requestCameraPermission(this)
            return
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>,
        results: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, results)
        if (CameraPermission.hasCameraPermission(this) == false) {
            Toast.makeText(this,
                "Camera permission is needed to run this application",
                Toast.LENGTH_LONG).show()
            if (CameraPermission.shouldShowRequestPermissionRationale(this) == false) {
                // Permission denied with checking "Do not ask again".
                CameraPermission.launchPermissionSettings(this)
            }
            finish()
        }
        // permission granted
        if(requestCode == CameraPermission.CAMERA_PERMISSION_CODE) {
            if(results[0] == PackageManager.PERMISSION_GRANTED) {
                // nothing for now
            }
        }
    }
}

class JSInterface(var context: Context){

    @JavascriptInterface
    fun showToast(message: String) {
        Toast.makeText(context, message, Toast.LENGTH_SHORT).show()
    }
}

class ChromeClient : WebChromeClient() {

    @RequiresApi(Build.VERSION_CODES.LOLLIPOP)
    override fun onPermissionRequest(request: PermissionRequest) {
        val requestedResources = request.resources
        for (r in requestedResources) {
            if (r == PermissionRequest.RESOURCE_VIDEO_CAPTURE) {
                request.grant(arrayOf(PermissionRequest.RESOURCE_VIDEO_CAPTURE))
                break
            }
        }
    }
    override fun onConsoleMessage(message: ConsoleMessage): Boolean {
        Log.d("WebAR", "${message.message()} -- From line " +
                "${message.lineNumber()} of ${message.sourceId()}")
        return true
    }
}