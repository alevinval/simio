package packager.api;

import packager.structs.Constants;
import packager.structs.FILE;
import packager.structs.FileBlock;

import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class FileApi {

    private MessageDigest digestor;

    public FileApi() throws NoSuchAlgorithmException {
        digestor = MessageDigest.getInstance("SHA-256");
    };

}
