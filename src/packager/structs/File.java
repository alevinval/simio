package packager.structs;

/*
	[ FILE ]
		HASH - SHA256(concatenate(BLOCKS[N].HASH))
		NAME - Original file name
		SIZE - Sizeof(BLOCKS)
		BLOCKS - Ordered array with the HASH of each FILE_BLOCK
 */
public class FILE {

    private byte[] HASH;
    private byte[] NAME;
    private int SIZE;
    private FileBlock[] BLOCKS;

    public FILE() {};

    public void setHash(byte[] hash) { HASH = hash; }
    public void setName(byte[] name) { NAME = name; }
    public void setSize(int size) { SIZE = size; }
    public void setBlocks(FileBlock[] blocks) { BLOCKS = blocks; }

    public byte[] getHash() { return HASH; }
    public byte[] getName() { return NAME; }
    public int getSize() { return SIZE; }
    public FileBlock[] getBlocks() { return BLOCKS; }

    public void print() {
        int blocks_count = 0;
        System.out.printf("FILE name: %s\n", new String(getName()));
        System.out.printf("FILE Hash: %s\n", Hash.toHex(getHash()));
        System.out.printf("FILE blocks: %s\n", getSize());

        for(FileBlock block : getBlocks()) {
            blocks_count++;
            System.out.printf("     Block %s\n", blocks_count);
            System.out.printf("     Size: %s\n", block.getSize());
            System.out.printf("     Hash: %s\n\n", Hash.toHex(block.getHash()));
        }
    }

}
