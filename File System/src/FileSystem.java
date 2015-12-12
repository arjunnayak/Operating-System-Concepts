import java.util.ArrayList;
import java.util.Random;
import java.util.Scanner;

public class FileSystem {
	
	private static int MEMORY_HARDWARE_LIMIT = 30; //sample hardware limit for memory
	private static int FILES_HARDWARE_LIMIT = 30; //sample hardware limit for files
	private static int BLOCK_SIZE = 4; //1 long = 4 bytes since data array holds 1 long per slot
	private static long data[]; //data array that maintains blocks of data used by files
	private static File[] files; //strictly maintains structure of file system, files will hold indexes within this array
	private static File root; //root file
	private static int iFreeFile; //index of next free file (within files array)
	private static int iFreeDataBlock; //index of next free data slot (within data array)
	private static int iCurrentFile; //index of current file/directory in CLI
	private static String owner; //owner string used pervasively
	private static Random random; //random generator for some file properties
	
	/*
	 * 1. Takes in owners name 
	 * 2. Initializes file system with owner name
	 * 3. Receives input and serves as a Command Line Interface (CLI) and performs commands/handles input errors
	 */
	public static void main(String args[]) 
	{	
		Scanner in = new Scanner(System.in);
		System.out.print("Enter name of owner: ");
		String owner = in.next(); //1
		initialize(owner); //2
		
		System.out.println("Initializing File System. Enter commands or q to quit.");
		System.out.print(owner + ": ");
		in.nextLine();
		reading: while(in.hasNextLine())
		{
			String input = in.nextLine();
			String[] command = input.trim().split("\\s"); //saves entire line entered in by user
			switch(command[0])  //3
			{
				case "q": //command for aborting CLI
					System.out.println("Aborting");
					break reading;
				case "currentd":
					System.out.println(files[iCurrentFile].name); //returns current file name
					break;
				case "maked":
					if(command.length > 1) { //if a second parameter was provided 
						String dirName = command[1];
						if(checkDir(dirName) < 0 ) { //if directory name does not already exist
							makeDirectory(dirName); //create directory with name provided
						} else {
							System.out.println(dirName + " already exists.");
						}
					} else {
						System.out.println("Needs parameters.");
					}
					break;
				case "chdir":
					if(command.length > 1) {
						String dir = command[1];
						int index = checkDir(dir); 
						if(index > 0) { //if there is a directory that exists within reach
							iCurrentFile = index;
						} else {
							System.out.println(dir + " is not a directory.");
						}
					} else { //if no second parameter is supplied, change to parent directory
						System.out.println("Going to parent directory");	
						File parent = files[iCurrentFile].parent;
						iCurrentFile = parent.index;
					}
					break;
				case "createf":
					if(command.length > 1) {
						String file = command[1]; //saves file name desired by user
						if(checkDir(file) < 0) { //if file name does not exist within reach, create file
							createFile(file); 
						} else {
							System.out.println(file + " already exists");
						}
					} else {
						System.out.println("Needs parameters.");
					}
					break;
				case "extendf":
					if(command.length > 1) {
						String fileName = command[1];
						int index = checkDir(fileName); //check to see if file exists
						if(index > 0) { 
							int extendBy = Integer.parseInt(command[2]); //save parameter
							extendFile(files[index], extendBy); //extend specific file by desired amount
						} else {
							System.out.println(fileName + " does not exist.");
						}
					} else {
						System.out.println("Needs parameters.");
					}
					break;
				case "truncf": //similar 
					if(command.length > 2) {
						String fileName = command[1];
						int index = checkDir(fileName);
						if(index > 1) {
							int truncBy = -1 * Integer.parseInt(command[2]); //saves number to trunc by
							extendFile(files[index], truncBy); //uses same method as extend, but extends by a negative number
						} else {
							System.out.println(fileName + " does not exist.");
						}
					} else {
						System.out.println("Needs parameters.");
					}
					break;
				case "deletefd":
					if(command.length > 1) {
						String fileName = command[1];
						int index = checkDir(fileName);
						if(index > 0) { //if file exists, use delete file method 
							deleteFile(index);
						} else {
							System.out.println(fileName + " does not exist.");
						}
					} else {
						System.out.println("Needs parameters.");
					}
					break;
				case "listd":
					if(command.length > 1) {
						String dir = command[1];
						int index = checkDir(dir);
						if(index > 0) { //if file exists, list children's names in CLI
							for(File f : files[index].children) {
								System.out.println(f.name + " ");
							}
						} else {
							System.out.println(dir + " is not a directory");
						}
					} else { //if no second parameter is supplied, list current directories children
						for(File f: files[iCurrentFile].children) {
							System.out.print(f.name + " ");
						}
						System.out.println();
					}
					break;
				case "listf":
					if(command.length > 1) {
						String fileName = command[1];
						int index = checkDir(fileName);
						if(index > 0) { //if file exists
							for(int i : files[index].dataBlockIndexes) { //print the indexes in the data array that it occupies
								System.out.print(i + " ");
							}
							System.out.println();
						} else {
							System.out.println(fileName + " does not exist.");
						}
					} else { //prints current directory's (from CLI) indexes occupied in data array
						System.out.println("Current file block addresses: ");
						for(int i : files[iCurrentFile].dataBlockIndexes) {
							System.out.print(i + " ");
						}
						System.out.println();
					}
					break;
				case "sizef":
					if(command.length > 1) {
						String fileName = command[1];
						int index = checkDir(fileName);
						if(index >= 0) { //if file exist, print number of blocks used and corresponding size in bytes
							System.out.println("Current file size: " + files[index].numBlocks + " blocks, " + 
									files[index].numBlocks * BLOCK_SIZE + " bytes");
						} else {
							System.out.println(fileName + " does not exist.");
						}
					} else{ //prints size for current directory if second parameter is not supplied
						System.out.println("Current file size: " + files[iCurrentFile].numBlocks + " blocks, " + 
											files[iCurrentFile].numBlocks * BLOCK_SIZE + " bytes");
					}
					break;
				case "movf":
					if(command.length > 2) {
						String file1 = command[1]; int index1 = checkDir(file1); 
						String file2 = command[2]; int index2 = checkDir(file2); //get names and indexes of both files
						if(index1 > 0) {
							if(index2 > 0) { //if both files exist
								moveFile(index1, index2); //use separate move file method
							} else {
								System.out.println(file2 + " does not exist.");
							}
						} else {
							System.out.println(file1 + " does not exist.");
						}
					} else {
						System.out.println("Needs parameters.");
					}
					break;
				case "listfb":
					for(int i = 0; i < data.length; i++) { //go through data array and print slots that are empty
						if(data[i] == 0) System.out.print(i + " ");
					}
					break;
				case "dumpfs":
					for(int i = 0; i < data.length; i++) { //print any non-empty slot in data array
						if(data[i] != 0) System.out.println(i + ": " + data[i]);
					}
					break;
				case "formatd":
					formatDisk();
					break;
				case "showCurrentFile":
					if(command.length > 1) {
						int index = checkDir(command[1]);
						if(index > 0) {
							File file = files[index];
							System.out.printf("Name: %s Type: %s Size: %d bytes Permission: %s Location: %d Parent-Name: %s %n", 
									file.name, file.type, file.numBlocks * BLOCK_SIZE, file.permission, file.index, file.parent.name);
						}
					} else {
						File file = files[iCurrentFile];
						System.out.printf("Name: %s Type: %s Size: %d bytes Permission: %s Location: %d Parent-Name: %s %n", 
											file.name, file.type, file.numBlocks * BLOCK_SIZE, file.permission, file.index, file.parent.name);
					}
					break;
				default: //user inputed incorrect command
					System.out.println("Incorrect Command.");
					break;
			}
			System.out.print(owner + ": ");
		}
	}
	
	/*
	 * Initialize the data (memory) and files (structure) arrays
	 * Create root file with owners name specified from user
	 * Update indexes of free file slots and free data block slots
	 */
	public static void initialize(String ownerStr) {  
		random = new Random();
		data = new long[MEMORY_HARDWARE_LIMIT];
		files = new File[FILES_HARDWARE_LIMIT];
		owner = ownerStr;
		iCurrentFile = 0;
		files[iCurrentFile] = new File("Home", "root", owner, 0, Permission.READ_WRITE, iCurrentFile, "dir", null, -1);
		root = files[iCurrentFile];
		iFreeFile = 1;
		iFreeDataBlock = 0;
	}
	
	/*
	 * General create file method
	 * Uses getRandomType() and getRandomPermission() to supply properties
	 * 1 Data array slot gets occupied, and the index to this slot is saved in the file's metadata
	 * File gets put into files array (for structure), its parent file is the current file in the CLI
	 * Update the size of root folders going upwards
	 */
	public static void createFile(String fileName) {
		File newFile = new File(fileName, getRandomType(), owner, 1, getRandomPermission(), iFreeFile, "file", files[iCurrentFile], iFreeDataBlock);
		data[iFreeDataBlock] = random.nextLong();
		iFreeDataBlock++;
		files[iFreeFile] = newFile;
		iFreeFile++;
		files[iCurrentFile].children.add(newFile);
		updateDataBlocks(newFile, newFile.numBlocks);
	}
	
	/*
	 * General create directory method
	 * Type is "directory", also gets random permission 
	 * The new directory's parent is the current file in the CLI 
	 * Does not occupy data block slot since it's just a directory
	 * Will still maintain size of itself based on the files it contains
	 */
	public static void makeDirectory(String dirName) {
		File newDir = new File(dirName, "dir", owner, 0, getRandomPermission(), iFreeFile, "dir", files[iCurrentFile], -1);
		files[iCurrentFile].children.add(newDir);
		files[iFreeFile] = newDir;
		iFreeFile++;
	}
	
	/*
	 * The contents of the current file are checked to see if the given file name exists already
	 * If it returns -1, then the name given does not exist within current directory
	 */
	public static int checkDir(String dirName) {
		for(File f : files[iCurrentFile].children) {
			if(f.name.equals(dirName)) {
				return f.index;
			}
		}
		return -1;
	}
	
	/*
	 * file: The file passed in is the one being extended
	 * extendBy: the amount the file will be extended by (can be a positive or negative integer [for trunc])
	 * Next free data block gets updated, as blocks become occupied with some data (represented as a long for example)
	 * Parent folders get updated using updateDataBlocks method
	 */
	public static void extendFile(File file, int extendBy) {
		file.numBlocks += extendBy;
		if(extendBy > 0) { //EXTENDF
			for(int i = iFreeDataBlock; i < iFreeDataBlock + extendBy; i++) {
				file.dataBlockIndexes.add(i);
				data[i] = random.nextLong();
			}
			iFreeDataBlock += extendBy;
		} else { //TRUNCF
			for(int i = 0; i < extendBy * -1; i++) {
				file.dataBlockIndexes.remove(file.dataBlockIndexes.size()-1);
			}
		}
		updateDataBlocks(file, extendBy);
	}
	
	/*
	 * Deletes file based on its index in the file structure array
	 * Clears data, removes children recursively, removes parent references, and finally removes self
	 */
	public static void deleteFile(int index) {
		//clear data being used
		if(!files[index].fileType.equals("dir")) { //if the file is not a directory, but a file that occupies slots
			for(int indexOfData : files[index].dataBlockIndexes) {
				data[indexOfData] = 0; // 'clears' data
			}
		}
		//remove children
		for(int i = 0; i < files[index].children.size(); i++) {
			deleteFile(files[index].children.get(i).index); //call delete file on its children as well (in recursive fashion)
		}
		//remove child reference from parent
		files[index].parent.children.remove(files[index]); 
		//remove self
		files[index] = null; //finally remove itself from file structure system
	}
	
	/*
	 * Takes index of item to be moved and index of destination in file-structure array
	 * Updates new and old parent/children references
	 * Updates size of destination (in blocks)
	 * Updated parent folders going upward is NOT needed
	 */
	public static void moveFile(int indexOfItem, int indexDest) {
		files[indexDest].children.add(files[indexOfItem]); //destination adds its new child
		files[indexOfItem].parent.children.remove(files[indexOfItem]); //removes old parent reference
		files[indexOfItem].parent = files[indexDest]; //adds new parent reference (destination)
		files[indexDest].numBlocks += files[indexOfItem].numBlocks; //updates destinations size (updated parent folders upwards not needed)
	}
	
	/*
	 * Completely formats file system
	 * Clears data, clears file references in array structure, and re-initializes
	 */
	public static void formatDisk() {
		for(int i = 0; i < data.length; i++) {
			data[i] = 0; //clears all data in data array
		} 
		for(int i = 0; i < files.length; i++) {
			files[i] = null; //sets all file references to emtpy
		}
		initialize(owner); //re-initializes with same owner name
	}
	
	/*
	 * Updates size of parent folders until we hit the root folder
	 */
	public static void updateDataBlocks(File currentFile, int size) {
		while(currentFile.parent != null) {
			currentFile = currentFile.parent;
			currentFile.numBlocks += size;
		}
	}
	
	/*
	 * Generates random file type 
	 */
	public static String getRandomType() {
		int x = random.nextInt(5);
		String result = "";
		switch(x) {
			case 0:
				result = "words";
				break;  
			case 1:
				result = "numbers";
				break;
			case 2:
				result = "photos";
				break;
			case 3:
				result = "music";
				break;
			case 4:
				result = "video";
				break;
		}
		return result;
	}
	
	/*
	 * Generates random permission. 
	 * Permission is an enum (see Permission.java)
	 */
	public static Permission getRandomPermission() {
		int x = random.nextInt(3);
		Permission p = null;
		switch(x) {
			case 0:
				p = Permission.READ;
				break;  
			case 1:
				p = Permission.READ_WRITE;
				break;
			case 2:
				p = Permission.SOFT_LINK;
				break;
			default:
				p = Permission.READ_WRITE;
				break;
		}
		return p;
	}
}
