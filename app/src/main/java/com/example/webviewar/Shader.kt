package com.example.webviewar

import android.opengl.GLES30.*
import java.io.ByteArrayOutputStream
import java.io.IOException
import java.io.InputStream
import java.lang.IllegalArgumentException
import java.nio.IntBuffer
import java.nio.charset.StandardCharsets

object Shader {

    fun createProgram(vertexPath: String, fragmentPath: String): Int {
        var ID = -1
        var vertShInStm: InputStream? = null
        var fragShInStm: InputStream? = null
        try {
            val values = tryCreateProgram(vertexPath, fragmentPath)
            ID = values.first
            fragShInStm = values.second
            vertShInStm = values.third
        } catch (e: IOException) {
            e.printStackTrace()
        } catch (e: Exception) {
            e.printStackTrace()
        } catch (e: IllegalArgumentException) {
            e.printStackTrace()
        } finally {
            vertShInStm?.close()
            fragShInStm?.close()
            return ID
        }
    }

    private fun tryCreateProgram(vertexPath: String, fragmentPath: String): Triple<Int, InputStream?, InputStream?> {
        val vertShInStm: InputStream? = globalAssets?.open(vertexPath)
        val fragShInStm: InputStream? = globalAssets?.open(fragmentPath)

        val vertexCode = readStream(vertShInStm!!)
        val fragmentCode = readStream(fragShInStm!!)

        val vertex = createCompileShader(GL_VERTEX_SHADER, vertexCode)
        val fragment = createCompileShader(GL_FRAGMENT_SHADER, fragmentCode)

        val ID: Int = glCreateProgram()
        glAttachShader(ID, vertex)
        glAttachShader(ID, fragment)

        glLinkProgram(ID)
        checkCompileErrors(ID, "program")

        glDeleteShader(vertex)
        glDeleteShader(fragment)
        return Triple(ID, fragShInStm, vertShInStm)
    }

    fun readStream(input: InputStream): String {

        val bais = ByteArrayOutputStream()

        // read data from input stream
        val buffer = ByteArray(1024)
        var length: Int
        while (input.read(buffer).also { length = it } != -1) {
            bais.write(buffer, 0, length)
        }
        // convert bytes stream to string
        return bais.toString(StandardCharsets.UTF_8.name())
    }

    fun createCompileShader(type: Int, shaderCode: String): Int {
        val shader = glCreateShader(type)
        if(type != GL_FRAGMENT_SHADER && type != GL_VERTEX_SHADER) {
            throw IllegalArgumentException("Shader type must be one of: GL_FRAGMENT_SHADER or GL_VERTEX_SHADER constant")
        }

        glShaderSource(shader, shaderCode)
        glCompileShader(shader)
        val ttype = if(type == GL_FRAGMENT_SHADER) "shader|fragment" else "shader|vertex"
        checkCompileErrors(shader, ttype)

        return shader
    }

    fun checkCompileErrors(shader: Int, type: String) {
        val success = IntBuffer.allocate(1)
        if(type.contains("program")) {
            glGetProgramiv(shader, GL_LINK_STATUS, success)
            if(success[0] == 0) {
                success.clear()
                val infoLog = glGetProgramInfoLog(shader)
                throw Exception("ERROR::PROGRAM::LINKING_FAILED\n Reason: \n $infoLog")
            }
        } else {
            glGetShaderiv(shader, GL_COMPILE_STATUS, success)
            if(success[0] == 0) {
                success.clear()
                val infoLog = glGetShaderInfoLog(shader)
                throw Exception("ERROR::SHADER::COMPILATION_FAILED::${type.uppercase()}\n Reason: \n $infoLog")
            }
        }
    }
}