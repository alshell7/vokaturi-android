package com.projects.alshell.vokaturi;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by Alshell7 @(Ashraf Khan Workstation)
 * 03:50 PM.
 * 05/Aug/2017
 */

public class VokaturiException extends Exception
{
    public static final int VOKATURI_OK = 0;
    public static final int VOKATURI_NOT_ENOUGH_SONORANCY = 2;
    public static final int VOKATURI_DENIED_PERMISSIONS = 3;
    public static final int VOKATURI_ERROR = -1;

    private int myErrorCode = VOKATURI_ERROR;

    private static final Map<Integer,String> errorCodes = new HashMap<>();

    public VokaturiException(String message)
    {
        super(message);
    }

    public VokaturiException (int code, String message) {
        super (message + " (" + code + ": " + errorCodes.get (code) + ")");
        this.myErrorCode = code;
    }

    private static void addErrorCode (int code, String def, String msg) {
        errorCodes.put (code, def + ": " + msg);
    }

    static {
        addErrorCode (VOKATURI_OK, "VOKATURI_OK", "Everything is seems fine.");
        addErrorCode (VOKATURI_ERROR, "VOKATURI_ERROR", "Generic Error.");
        addErrorCode (VOKATURI_NOT_ENOUGH_SONORANCY, "VOKATURI_NOT_ENOUGH_SONORANCY", "Not enough sonorancy to determine emotions");
        addErrorCode (VOKATURI_DENIED_PERMISSIONS, "VOKATURI_DENIED_PERMISSIONS", "File Read Write/Recording Audio permissions not granted");
    }

    public void __jni_setLocation(String functionName, String file, int line) {
        addStackTraceElement (this, functionName, file, line);
    }

    private void addStackTraceElement (Throwable t, String functionName, String file, int line) {

        StackTraceElement[] currentStack =  t.getStackTrace ();
        StackTraceElement[] newStack =  new StackTraceElement[currentStack.length + 1];

        System.arraycopy (currentStack, 0, newStack, 1, currentStack.length);

        file = file.replace ('\\', '/');
        if (file.lastIndexOf ('/') > -1) {
            file = file.substring (file.lastIndexOf ('/') + 1);
        }

        newStack[0] = new StackTraceElement ("<native>", functionName, file, line);
        t.setStackTrace (newStack);
    }

    public int getErrorCode()
    {
        return myErrorCode;
    }
}
