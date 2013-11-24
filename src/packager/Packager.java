package packager;

import packager.api.BlocksApi;
import packager.api.FileApi;
import packager.structs.FILE;
import packager.structs.FileBlock;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Packager {

    private MessageDigest digestor;
    private BlocksApi blocksApi;
    private FileApi fileApi;

    public Packager() throws NoSuchAlgorithmException {
        digestor  = MessageDigest.getInstance("SHA-256");
        blocksApi = new BlocksApi();
        fileApi = new FileApi();
    };

    public FILE toFile(File oFile) throws IOException, NoSuchAlgorithmException {
        FILE file = new FILE();
        FileBlock[] blocks;
        byte[] blocks_hash;

        blocks = blocksApi.getBlocks(oFile);
        blocks_hash = blocksApi.getBlocksHash(blocks);
        file.setHash(blocks_hash);
        file.setName(oFile.getName().getBytes());
        file.setSize(blocks.length);
        file.setBlocks(blocks);

        return file;
    }

    public void duplicate(FILE file, String fname) throws IOException {
        java.io.File duplicate = new java.io.File(fname);
        duplicate.createNewFile();

        FileOutputStream stream = new FileOutputStream(duplicate);

        int written_bytes = 0;
        for(FileBlock block : file.getBlocks()) {
            stream.write(block.getData(), 0, block.getSize());
            written_bytes += block.getSize();
        }

        stream.close();
    }
}
