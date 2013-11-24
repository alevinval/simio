package packager.structs;

/*
	[ OBJECT ] ( Holds a FILE or a DIR, specified by the TYPE field )
		TYPE - TYPE_DIR or TYPE_FILE
		SIZE - Sizeof(DATA)
		DATA - Serialized  FILE or DIR

 */
public class Object {
    private int TYPE;
    private int SIZE;
    private byte[] DATA;

    public Object() {};

    public void setType(int type) { TYPE = type; }
    public void setSize(int size) { SIZE = size; }
    public void setData(byte[] data){ DATA = data; }

    public int getType() { return TYPE; }
    public int getSize() { return SIZE; }
    public byte[] getData() { return DATA; }
}
