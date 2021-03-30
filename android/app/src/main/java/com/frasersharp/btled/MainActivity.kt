package com.frasersharp.btled

import android.graphics.Color
import android.os.Build
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import kotlin.math.floor


class MainActivity : AppCompatActivity() {
    private var bluetoothSerial: BluetoothSerial? = null

    override fun onPause() {
        super.onPause()
        bluetoothSerial!!.onPause()
    }

    override fun onResume() {
        super.onResume()
        bluetoothSerial!!.onResume()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        connectBluetooth()
    }

    fun connectBluetooth() {
        bluetoothSerial =
            BluetoothSerial(this.baseContext,
                BluetoothSerial.MessageHandler { bufferSize, buffer -> doRead(bufferSize, buffer) },
                "HC"
            )

        bluetoothSerial!!.connect()
    }

    private fun doRead(bufferSize: Int, buffer: ByteArray): Int {
        var msg = ""
        for (i in 0 until bufferSize) {
            msg += buffer[i].toChar()
        }
        Toast.makeText(this, "BT IN: $msg", Toast.LENGTH_SHORT).show()
        return 0
    }

    private fun sendColourList(animationSteps: Array<AnimationStep>) {
        val msgOut = ByteArray(5 * animationSteps.size + 2)

        msgOut[0] = 0x23
        msgOut[1] = 0x32

        for (i in animationSteps.indices) {
            val color = animationSteps[i].color
            msgOut[5 * i + 2] = floor(color.red() * 255).toByte()
            msgOut[5 * i + 2 + 1] = floor(color.green() * 255).toByte()
            msgOut[5 * i + 2 + 2] = floor(color.blue() * 255).toByte()
            msgOut[5 * i + 2 + 3] = animationSteps[i].duration.toByte()
            msgOut[5 * i + 2 + 4] =
                (if (i < animationSteps.size - 1) 0x07 else 0x00).toByte()
        }

        try {
            var msg = ""
            for (b in msgOut) {
                msg += b.toChar()
            }
            bluetoothSerial!!.write(msgOut)
        } catch (e: Exception) {
            e.printStackTrace()
        }

    }

    fun patternButtonOff(view: View) {
        sendColourList(arrayOf(
            AnimationStep(Color.valueOf(0.0f, 0.0f, 0.0f), 100)
        ));
    }

    fun patternButtonColdWhite(view: View) {
        sendColourList(arrayOf(
            AnimationStep(Color.valueOf(1.0f, 1.0f, 1.0f), 100)
        ));
    }

    fun patternButtonWarmWhite(view: View) {
        sendColourList(arrayOf(
            AnimationStep(Color.valueOf(1.0f, 0.5f, 0.1f), 100)
        ));
    }

    fun patternButtonPurpleFade(view: View) {
        sendColourList(arrayOf(
            AnimationStep(Color.valueOf(1.0f, 0.0f, 1.0f), 100),
            AnimationStep(Color.valueOf(0.3f, 0.0f, 1.0f), 100),
            AnimationStep(Color.valueOf(1.0f, 0.0f, 0.0f), 100),
            AnimationStep(Color.valueOf(0.6f, 0.0f, 1.0f), 100),
            AnimationStep(Color.valueOf(1.0f, 0.0f, 0.6f), 100),
            AnimationStep(Color.valueOf(0.2f, 0.0f, 1.0f), 100)
        ));
    }

    fun patternButtonCyanFade(view: View) {
        sendColourList(arrayOf(
            AnimationStep(Color.valueOf(0.0f, 1.0f, 1.0f), 100),
            AnimationStep(Color.valueOf(0.0f, 0.4f, 1.0f), 100),
            AnimationStep(Color.valueOf(0.0f, 1.0f, 0.3f), 100),
            AnimationStep(Color.valueOf(0.0f, 0.6f, 0.8f), 100),
            AnimationStep(Color.valueOf(0.0f, 1.0f, 0.1f), 100),
            AnimationStep(Color.valueOf(0.0f, 0.0f, 1.0f), 100)
        ));
    }

    fun patternButtonCandle(view: View) {
        sendColourList(arrayOf(
            AnimationStep(Color.valueOf(0.5f, 0.2f, 0.0f), 100),
            AnimationStep(Color.valueOf(0.7f, 0.3f, 0.0f), 100),
            AnimationStep(Color.valueOf(0.3f, 0.05f, 0.0f), 100),
            AnimationStep(Color.valueOf(0.5f, 0.1f, 0.0f), 100),
            AnimationStep(Color.valueOf(0.6f, 0.2f, 0.0f), 100),
            AnimationStep(Color.valueOf(0.5f, 0.2f, 0.0f), 100),
            AnimationStep(Color.valueOf(0.7f, 0.2f, 0.0f), 100)
        ));
    }

    fun patternButtonEvilPulse(view: View) {
        sendColourList(arrayOf(
            AnimationStep(Color.valueOf(1.0f, 0.0f, 0.0f), 1),
            AnimationStep(Color.valueOf(0.0f, 0.0f, 0.0f), 1)
        ));
    }
}