package com.frasersharp.btled

import android.graphics.Color
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.graphics.blue
import androidx.core.graphics.green
import androidx.core.graphics.red
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

        val callback = object :
            SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seek: SeekBar, progress: Int, fromUser: Boolean) {
                updateColourPreview()
            }

            override fun onStartTrackingTouch(seek: SeekBar) {

                updateColourPreview()
            }

            override fun onStopTrackingTouch(seek: SeekBar) {
                updateColourPreview()}
        }
        val hueBar = findViewById<View>(R.id.seekBarHue) as SeekBar
        val satBar = findViewById<View>(R.id.seekBarSaturation) as SeekBar
        val intBar = findViewById<View>(R.id.seekBarIntensity) as SeekBar

        hueBar.setOnSeekBarChangeListener(callback)
        satBar.setOnSeekBarChangeListener(callback)
        intBar.setOnSeekBarChangeListener(callback)
    }

    fun updateColourPreview() {
        val hue = findViewById<SeekBar>(R.id.seekBarHue).progress / 10.0f
        val saturation = findViewById<SeekBar>(R.id.seekBarSaturation).progress / 1000.0f
        val intensity = findViewById<SeekBar>(R.id.seekBarIntensity).progress / 1000.0f

        val colour = Color.HSVToColor(255, floatArrayOf(hue, saturation, intensity))
        Log.w("BTLED", "h $hue s $saturation i $intensity")
        Log.w("BTLED", "r ${colour.red} g ${colour.green} b ${colour.blue}")

        val button = findViewById<Button>(R.id.setColourButton)
        button.setBackgroundColor(colour)
    }

    fun sendSingleColour(view: View) {
        val hue = findViewById<SeekBar>(R.id.seekBarHue).progress / 10.0f
        val saturation = findViewById<SeekBar>(R.id.seekBarSaturation).progress / 1000.0f
        val intensity = findViewById<SeekBar>(R.id.seekBarIntensity).progress / 1000.0f

        val colour = Color.HSVToColor(floatArrayOf(hue, saturation, intensity))
        sendColourList(
            arrayOf(
                AnimationStep(Color.valueOf(colour), 100)
            )
        )
    }

    private fun connectBluetooth() {
        bluetoothSerial =
            BluetoothSerial(
                this.baseContext,
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
        val msgOut = ByteArray(7 * animationSteps.size + 2)

        msgOut[0] = 0x23
        msgOut[1] = 0x32

        for (i in animationSteps.indices) {
            val color = animationSteps[i].color
            msgOut[7 * i + 2] = floor(color.red() * 255).toByte()
            msgOut[7 * i + 2 + 1] = floor(color.green() * 255).toByte()
            msgOut[7 * i + 2 + 2] = floor(color.blue() * 255).toByte()
            msgOut[7 * i + 2 + 3] = animationSteps[i].duration.toByte()
            msgOut[7 * i + 2 + 4] = animationSteps[i].flickerAmplitude.toByte()
            msgOut[7 * i + 2 + 5] = animationSteps[i].flickerFrequency.toByte()
            msgOut[7 * i + 2 + 6] =
                (if (i < animationSteps.size - 1) 0x07 else 0x00).toByte()
        }

        var logline = ""
        for (m in msgOut) {
            logline += m.toString(16)
            logline += ' '
        }
        Log.i("BTLED", logline)

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
        sendColourList(
            arrayOf(
                AnimationStep(Color.valueOf(0.0f, 0.0f, 0.0f), 100)
            )
        );
    }

    fun patternButtonColdWhite(view: View) {
        sendColourList(
            arrayOf(
                AnimationStep(Color.valueOf(1.0f, 1.0f, 1.0f), 100)
            )
        );
    }

    fun patternButtonWarmWhite(view: View) {
        sendColourList(
            arrayOf(
                AnimationStep(Color.valueOf(1.0f, 0.35f, 0.05f), 100, 20, 3000)
            )
        );
    }

    fun patternButtonPurpleFade(view: View) {
        sendColourList(
            arrayOf(
                AnimationStep(Color.valueOf(1.0f, 0.0f, 1.0f), 100, 50, 50),
                AnimationStep(Color.valueOf(0.3f, 0.0f, 1.0f), 100, 80, 50),
                AnimationStep(Color.valueOf(1.0f, 0.0f, 0.0f), 100, 50, 50),
                AnimationStep(Color.valueOf(0.6f, 0.0f, 1.0f), 100, 50, 50),
                AnimationStep(Color.valueOf(1.0f, 0.0f, 0.6f), 100, 50, 50),
                AnimationStep(Color.valueOf(0.2f, 0.0f, 1.0f), 100, 50, 50)
            )
        );
    }

    fun patternButtonCyanFade(view: View) {
        sendColourList(
            arrayOf(
                AnimationStep(Color.valueOf(0.0f, 1.0f, 1.0f), 100),
                AnimationStep(Color.valueOf(0.0f, 0.4f, 1.0f), 100),
                AnimationStep(Color.valueOf(0.0f, 1.0f, 0.3f), 100),
                AnimationStep(Color.valueOf(0.0f, 0.6f, 0.8f), 100),
                AnimationStep(Color.valueOf(0.0f, 1.0f, 0.1f), 100),
                AnimationStep(Color.valueOf(0.0f, 0.0f, 1.0f), 100)
            )
        );
    }

    fun patternButtonCandle(view: View) {
        sendColourList(
            arrayOf(
                AnimationStep(Color.valueOf(0.5f, 0.1f, 0.0f), 100, 30, 100),
                AnimationStep(Color.valueOf(0.7f, 0.2f, 0.0f), 100, 20, 100),
                AnimationStep(Color.valueOf(0.3f, 0.05f, 0.0f), 100, 35, 100),
                AnimationStep(Color.valueOf(0.5f, 0.1f, 0.0f), 100, 20, 100),
                AnimationStep(Color.valueOf(0.6f, 0.2f, 0.0f), 100, 10, 100),
                AnimationStep(Color.valueOf(0.5f, 0.1f, 0.0f), 100, 20, 100),
                AnimationStep(Color.valueOf(0.7f, 0.2f, 0.0f), 100, 30, 100)
            )
        );
    }

    fun patternButtonEvilPulse(view: View) {
        sendColourList(
            arrayOf(
                AnimationStep(Color.valueOf(1.0f, 0.0f, 0.0f), 1),
                AnimationStep(Color.valueOf(0.0f, 0.0f, 0.0f), 1)
            )
        );
    }
}