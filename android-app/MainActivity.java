package com.example.zoulm.myapplication;

import android.os.*;
//import android.os.Handler;
//import android.os.Message;
//import android.os.Looper;
import android.util.Log;

import android.net.Uri;
import android.support.v7.app.AppCompatActivity;
//import android.os.Bundle;
import android.view.MotionEvent;

import com.google.android.gms.appindexing.Action;
import com.google.android.gms.appindexing.AppIndex;
import com.google.android.gms.common.api.GoogleApiClient;

import java.util.Timer;
import java.util.TimerTask;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;


public class MainActivity extends AppCompatActivity {

    /**
     * ATTENTION: This was auto-generated to implement the App Indexing API.
     * See https://g.co/AppIndexing/AndroidStudio for more information.
     */
    private GoogleApiClient client;

    @Override
    public void onStart() {
        super.onStart();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client.connect();
        Action viewAction = Action.newAction(
                Action.TYPE_VIEW, // TODO: choose an action type.
                "Main Page", // TODO: Define a title for the content shown.
                // TODO: If you have web page content that matches this app activity's content,
                // make sure this auto-generated web page URL is correct.
                // Otherwise, set the URL to null.
                Uri.parse("http://host/path"),
                // TODO: Make sure this auto-generated app URL is correct.
                Uri.parse("android-app://com.example.zoulm.myapplication/http/host/path")
        );
        AppIndex.AppIndexApi.start(client, viewAction);
    }

    @Override
    public void onStop() {
        super.onStop();

        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        Action viewAction = Action.newAction(
                Action.TYPE_VIEW, // TODO: choose an action type.
                "Main Page", // TODO: Define a title for the content shown.
                // TODO: If you have web page content that matches this app activity's content,
                // make sure this auto-generated web page URL is correct.
                // Otherwise, set the URL to null.
                Uri.parse("http://host/path"),
                // TODO: Make sure this auto-generated app URL is correct.
                Uri.parse("android-app://com.example.zoulm.myapplication/http/host/path")
        );
        AppIndex.AppIndexApi.end(client, viewAction);
        client.disconnect();
    }

    String LOG_TAG = "zoulm";

    private udpConnect myUdpConnect;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // ATTENTION: This was auto-generated to implement the App Indexing API.
        // See https://g.co/AppIndexing/AndroidStudio for more information.
        client = new GoogleApiClient.Builder(this).addApi(AppIndex.API).build();

        myUdpConnect = new udpConnect();

        new Thread(myUdpConnect).start();  /* 启动用于发送数据的线程 */
    }

    private long downTime = 0;
    private long upTime = 0;

    private int pressX = 0;
    private int pressY = 0;

    private int diffX = 0;
    private int diffY = 0;

    private int downCount = 0;

    private boolean isMove = false;

    private class ReportData {
        public int button;
        public int rel_x;
        public int rel_y;
        public int rel_wheel;

        ReportData(int button, int rel_x, int rel_y, int rel_wheel) {
            this.button = button;
            this.rel_x = rel_x;
            this.rel_y = rel_y;
            this.rel_wheel = rel_wheel;
        }

        byte[] intToByte(int arg){
            byte[] data = new byte[4];

    		/*data[0] = (byte) (arg & 0xff);
    		data[1] = (byte) (arg >> 8 & 0xff);
    		data[2] = (byte) (arg >> 16 & 0xff);
    		data[3] = (byte) (arg >> 24 & 0xff);*/

            data[3] = (byte) (arg & 0xff);
            data[2] = (byte) (arg >> 8 & 0xff);
            data[1] = (byte) (arg >> 16 & 0xff);
            data[0] = (byte) (arg >> 24 & 0xff);

            return data;
        }

        public byte[] toByte() {
            int i = 0;
            byte[] temp;
            byte[] data = new byte[16];
            int[] reportData = {button, rel_x, rel_y, rel_wheel};

            for (int j = 0; j < reportData.length; j++) {

                temp = intToByte(reportData[j]);

                for (int j2 = 0; j2 < temp.length; i++, j2++) {
                    data[i] = temp[j2];
                }
            }
            return data;
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {

        long timer = System.currentTimeMillis();

        Log.i(LOG_TAG, "event: " + String.valueOf(event.getAction()));

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:  /* 第一个点按下 */

                downCount = 1;

                downTime = timer;

                pressX = (int) event.getX();
                pressY = (int) event.getY();

                Log.i(LOG_TAG, "action down!!!");

                break;
            case MotionEvent.ACTION_UP:   /* 最后一个点释放 */

                Log.i(LOG_TAG, "action up!!! downCount:" + String.valueOf(downCount) + "  ismove:" + String.valueOf(isMove));

                upTime = timer;

                if (!isMove) {
                    if (1 == downCount)
                    {
                        ReportData reportData = new ReportData(0x01000000, 0, 0, 0);

                        Message message = myUdpConnect.revHandler.obtainMessage(udpConnect.MSG_SEND_DATA, reportData);

                        myUdpConnect.revHandler.sendMessage(message);

                        ReportData reportData1 = new ReportData(0, 0, 0, 0);

                        Message message1 = myUdpConnect.revHandler.obtainMessage(udpConnect.MSG_SEND_DATA, reportData1);

                        myUdpConnect.revHandler.sendMessage(message1);
                    }
                    else if (3 == downCount)
                    {
                        ReportData reportData = new ReportData(0x02000000, 0, 0, 0);

                        Message message = myUdpConnect.revHandler.obtainMessage(udpConnect.MSG_SEND_DATA, reportData);

                        myUdpConnect.revHandler.sendMessage(message);

                        ReportData reportData1 = new ReportData(0, 0, 0, 0);

                        Message message1 = myUdpConnect.revHandler.obtainMessage(udpConnect.MSG_SEND_DATA, reportData1);

                        myUdpConnect.revHandler.sendMessage(message1);
                    }
                }

                isMove = false;
                break;
            case MotionEvent.ACTION_POINTER_DOWN: /* 之后的点按下 */
                downCount++;

                Log.i(LOG_TAG, "downCount: " + String.valueOf(downCount));
                break;
            case MotionEvent.ACTION_POINTER_2_DOWN:
                downCount = 2;

                Log.i(LOG_TAG, "downCount: " + String.valueOf(downCount));
                break;
            case MotionEvent.ACTION_POINTER_3_DOWN:
                downCount = 3;

                Log.i(LOG_TAG, "downCount: " + String.valueOf(downCount));
                break;
            case MotionEvent.ACTION_POINTER_UP:  /* 之后的点释放 */
                break;
            case MotionEvent.ACTION_MOVE: /* 发生移动 */

                int x = (int) event.getX();
                int y = (int) event.getY();

                diffX = x - pressX;
                diffY = y - pressY;

                pressX = x;
                pressY = y;

                if (!isMove) {
                    if ((Math.abs(diffX) > 5) || (Math.abs(diffY) > 5)) {

                        if(1 == downCount)
                        {
                            isMove = true;
                        }
                    }
                } else {
                    Log.i(LOG_TAG, "event move: " + String.valueOf(diffX) + ", " + String.valueOf(diffY));

                    ReportData reportData = new ReportData(0, diffX, diffY, 0);

                    Message message = myUdpConnect.revHandler.obtainMessage(udpConnect.MSG_SEND_DATA, reportData);

                    myUdpConnect.revHandler.sendMessage(message);
                }
                break;
            default:
                break;
        }

        return super.onTouchEvent(event);
    }

    public class udpConnect implements Runnable {

        private static final int SERVER_PORT = 6669;

        private  byte HostIp[] = new byte[] { (byte)192, (byte)168, 1, (byte)188};

        //private byte HostIp[] = new byte[] { (byte)127, 0, 0, 1};

        private DatagramSocket dSocket = null;

        private InetAddress host = null;

        public Handler revHandler;

        public final static int MSG_SEND_DATA = 0x01;

        @Override
        public void run() {

            try {
                host =  InetAddress.getByAddress(HostIp);
            } catch (UnknownHostException e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }

            Log.i(LOG_TAG, "create InetAddress suc!!!");

            try {
                dSocket = new DatagramSocket(SERVER_PORT);
            } catch (SocketException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            Log.i(LOG_TAG, "create udp socket suc!!!");

            Looper.prepare();

            revHandler = new Handler() {

                @Override
                public void handleMessage(Message msg) {
                    switch (msg.what) {
                        case MSG_SEND_DATA:
                            Log.i(LOG_TAG, "udp send data!!");

                            ReportData reportData = (ReportData) msg.obj;

                            byte[] temp = reportData.toByte();

                            DatagramPacket dSendPacket = new DatagramPacket(temp, temp.length, host, SERVER_PORT);

                            String tempString = new String();

                            for (int i = 0; i < temp.length; i++) {
                                tempString += Integer.toHexString(temp[i] & 0xff);
                                tempString += " ";
                            }

                            Log.i("udp", "data: " + tempString);

                            try {
                                dSocket.send(dSendPacket);
                            } catch (IOException e) {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }

                            break;
                        default:
                            break;
                    }
                };
            };

            Looper.loop();
        }
    }
}


