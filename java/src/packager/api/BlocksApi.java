package packager.api;

import packager.structs.Constants;
import packager.structs.FILE;
import packager.structs.FileBlock;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class BlocksApi {

    private MessageDigest digestor;

    public BlocksApi() throws NoSuchAlgorithmException {
        digestor = MessageDigest.getInstance("SHA-256");
    }

    private int getBlocksSize(File file) {
        long fsize = file.length();
        return (int) fsize / Constants.BLOCK_SIZE + 1;
    }

    private byte[] getBlockHash(byte[] data, int len) {
        digestor.reset();
        digestor.update(data, 0, len);
        return digestor.digest();
    }

    private FileBlock createBlock(byte hash[], int size, byte data[]) {
        FileBlock block = new FileBlock();
        block.setHash(hash);
        block.setSize(size);
        block.setData(data);
        return block;
    }

    public FileBlock[] getBlocks(File file) throws IOException, NoSuchAlgorithmException {
        FileInputStream stream = new FileInputStream(file);
        int blocks_size = getBlocksSize(file);
        FileBlock[] blocks = new FileBlock[blocks_size];
        int next = 0;
        while (next < blocks_size) {
            byte data[] = new byte[Constants.BLOCK_SIZE];
            int data_size = stream.read(data);
            byte[] data_hash = getBlockHash(data, data_size);
            blocks[next] = createBlock(data_hash, data_size, data);
            next++;
        }
        return blocks;
    }

    public byte[] getBlocksHash(FileBlock[] blocks) {
        ByteBuffer buffer = ByteBuffer.allocate(blocks.length * Constants.SHA256_SIZE);
        for (FileBlock f : blocks)
            buffer.put(f.getHash());
        digestor.reset();
        digestor.update(buffer.array());
        return digestor.digest();
    }
}
