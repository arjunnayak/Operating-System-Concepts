import java.util.ArrayList;

public class File {
	
	public String name; //file name (for human purposes!)
	public String type; //file type (ie words, numbers, photos) - randomly generated
	public String owner; //owner or file
	public int numBlocks; //size in number of blocks (size in bytes = numBlocks * BLOCK_SIZE)
	public Permission permission; //permission of file
	public int index; //location of file in file array structure (array files)
	public String fileType; //either "dir" or "file", used for internal command purposes
	public File parent; //reference to parent file
	public ArrayList<File> children; //reference to children files
	public ArrayList<Integer> dataBlockIndexes; //list of data blocks it occupies on disk

	public File(String name, String type, String owner, int numBlocks, Permission p,
				int index, String fileType, File parent, int initialDataBlock) 
	{
		this.name = name;
		this.type = type;
		this.owner = owner;
		this.numBlocks = numBlocks;
		this.permission = p;
		this.index = index;
		this.fileType = fileType;
		this.parent = parent;
		this.children = new ArrayList<File>(); //when a file is created, it has no children, so we just initialize list
		this.dataBlockIndexes = new ArrayList<Integer>(); 
		this.dataBlockIndexes.add(initialDataBlock);
	}
}
