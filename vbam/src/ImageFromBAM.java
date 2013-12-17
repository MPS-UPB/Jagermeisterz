import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;


public class ImageFromBAM {

	public String resultImagePath;
	public BufferedImage image;
	public BufferedImage confTIF;


	//HashMap <Integer, ArrayList<Integer>> conf = new HashMap <Integer, ArrayList<Integer>>();

	public ImageFromBAM (String path) {
		this.resultImagePath = path;
	}

	public ImageFromBAM(BufferedImage image) {
		this.image = image;
		confTIF = null;
	}
	
	public void readImage() throws IOException {
		File f = new File(resultImagePath);
		image = ImageIO.read(f);
	}

	public void writeImage() throws IOException{
		ImageIO.write(image, "tif", new File(resultImagePath.replace(".tif", "_output.tif")));
	}

	public void readConf() throws IOException {
		//String filepath = resultImagePath.replace(".tif", "_conf.tif");
		System.out.println(resultImagePath);
		File f = new File(resultImagePath);
		confTIF = ImageIO.read(f);	
	}

}
