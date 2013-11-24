package packager.structs;

/*
	[ FILE_BLOCK ] ( Data fragment of a file )
		HASH - SHA256(DATA)
		SIZE - Sizeof(DATA)
		DATA - Raw data

 */
public class FileBlock {
    private byte[] HASH;
    private int SIZE;
    private byte[] DATA;

    public FileBlock() {};

    public void setHash(byte[] hash) { HASH = hash; }
    public void setSize(int size) { SIZE = size; }
    public void setData(byte[] data) { DATA = data; }

    public byte[] getHash() { return HASH; }
    public int getSize() { return SIZE; }
    public byte[] getData() { return DATA; }
}
