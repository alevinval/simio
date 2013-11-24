package packager.structs;

/*
	[ DIR ]
		NAME - Original dir name
		HASH - SHA256(concatenate(OBJECTS[N].DATA.NAME))
		OBJECTS - Array with the objects inside the directory
 */
public class Dir {
    private char[] NAME;
    private byte[] HASH;
    private Object[] OBJECTS;

    public Dir() {};

    public void setName(char[] name) { NAME = name; }
    public void setHash(byte[] hash) { HASH = hash; }
    public void setObjects(Object[] objects) { OBJECTS = objects; }

    public char[] getName() { return NAME; }
    public byte[] getHash() { return HASH; }
    public Object[] getObjects() { return OBJECTS; }

}
