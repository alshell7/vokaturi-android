package com.projects.alshell.vokaturi;

import java.util.HashMap;
import java.util.Map;

/**
 * An exception thrown by the library (from both native or java implementation) indicating any serious problem.
 */
public class VokaturiException extends Exception
{

    /**
     * Generic error.
     */
    public static final int VOKATURI_ERROR = -1;

    /**
     * Not enough sonorancy to determine emotions.
     * <p>
     *    If the speech sound that is produced is not continuous, and has turbulent audio.
     * </p>
     */
    public static final int VOKATURI_NOT_ENOUGH_SONORANCY = 2;

    /**
     * File Read Write/Recording Audio permissions not granted for the application.
     */
    public static final int VOKATURI_DENIED_PERMISSIONS = 3;


    /**
     * The given file is not of a valid WAV format.
     */
    public static final int VOKATURI_INVALID_WAV_FILE = 4;


    private int myErrorCode = VOKATURI_ERROR;

    private static final Map<Integer,String> errorCodes = new HashMap<>();

    public VokaturiException(String message)
    {
        super(message);
    }

    /**
     * Constructor with error code and message
     * @param code Error code
     * @param message cause of the exception
     */
    public VokaturiException (int code, String message) {
        super (message + " (" + code + ": " + errorCodes.get (code) + ")");
        this.myErrorCode = code;
    }

    private static void addErrorCode (int code, String def, String msg) {
        errorCodes.put (code, def + ": " + msg);
    }

    static {
        addErrorCode (VOKATURI_ERROR, "VOKATURI_ERROR", "Generic error");
        addErrorCode (VOKATURI_NOT_ENOUGH_SONORANCY, "VOKATURI_NOT_ENOUGH_SONORANCY", "Not enough sonorancy to determine emotions");
        addErrorCode (VOKATURI_DENIED_PERMISSIONS, "VOKATURI_DENIED_PERMISSIONS", "File Read Write/Recording Audio permissions not granted");
        addErrorCode (VOKATURI_INVALID_WAV_FILE, "VOKATURI_INVALID_WAV_FILE", "The given file is not of a valid WAV format");
    }

    /**
     * This is used by the native code to call the exception and set information
     */
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

    /**
     * Gets the error code of the exception thrown
     * @return Error code
     */
    public int getErrorCode()
    {
        return myErrorCode;
    }
}
