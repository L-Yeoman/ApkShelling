package com.lybin.android.xposed.shelling;
import static de.robv.android.xposed.XposedBridge.log;

public class Native {
    static {
        try {
            System.load("/data/local/tmp/libshelling.so");
        } catch (Throwable t) {
            log(t.getMessage());
        }
    }
/*   static {
       try{
           System.loadLibrary("shelling");

       }catch (Throwable t){
           Log.i("Native","load err:"+t.getMessage());
       }
   }*/

    public static native void test(String packageName);
}
