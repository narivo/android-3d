package com.example.webviewar

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_home.*

class Home : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_home)

        animation3D.setOnClickListener {
            startActivity(NativeLibActivity::class.java)
        }
        ar_button.setOnClickListener {
            startActivity(ARActivity::class.java)
        }
    }
}

fun <T> AppCompatActivity.startActivity(activityClass: Class<T>) {
    val i = Intent(this, activityClass)
    startActivity(i)
}