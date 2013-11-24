package main;

import packager.Packager;

import packager.structs.FILE;

import java.io.File;
import java.io.IOException;
import java.security.NoSuchAlgorithmException;

public class Main {

    public static void main(String[] args) throws NoSuchAlgorithmException, IOException {
        File f = new File("./src/packager/api/BlocksApi.java");

        Packager packager = new Packager();
        FILE file = packager.toFile(f);

        packager.duplicate(file, "copia.txt");

        file.print();
    }
}
