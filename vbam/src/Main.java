import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.ObjectInputStream.GetField;
import java.util.ArrayList;
import java.util.Arrays;

import javax.imageio.ImageIO;
import javax.imageio.spi.IIORegistry;

public class Main {

	static String imgExtension = ".tif";

	public static int getSize(String path) throws IOException {
		BufferedImage img = ImageIO.read(new File(path));
		int size = img.getHeight() * img.getWidth();
		return size;
	}

	public static BufferedImage getBenchmark(String dirPath, String testImgPath) {
		String execPath = dirPath + "\\benchmark\\Noob.exe";
		try {
			System.out.println("---- getBenchmark: " + execPath + " " + testImgPath);
			Process process = new ProcessBuilder(execPath,
					testImgPath).start();
			InputStream is = process.getInputStream();
			InputStreamReader isr = new InputStreamReader(is);
			BufferedReader br = new BufferedReader(isr);
			String line;

			while ((line = br.readLine()) != null) {
				System.out.println(line);
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}



		String benchmarkImgPath = execPath + "_Black_and_White.tif";
		BufferedImage img = null;
		try {
			img = ImageIO.read(new File(benchmarkImgPath));
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return img;
	}

	public static void main(String[] args) throws IOException {
		// TODO Auto-generated method stub

		if (args.length != 6) {
			System.out.println("Wrong arguments!");
			return;
		}

		//Add .tif file types to ImageIO
		IIORegistry registry = IIORegistry.getDefaultInstance();
		registry.registerServiceProvider(new com.sun.media.imageioimpl.plugins.tiff.TIFFImageWriterSpi());
		registry.registerServiceProvider(new com.sun.media.imageioimpl.plugins.tiff.TIFFImageReaderSpi());

		int timeout = Integer.parseInt(args[0]);
		int iniTimeout = Integer.parseInt(args[1]);

		String execDirPath = args[2];
		String testImgPath = args[3];
		String BAMresultDirPath = args[4];

		String resultImgFN = args[5];

		int imgSize = getSize(testImgPath);

		final int timerForBAM = timeout * imgSize + iniTimeout;

		BufferedImage n00bImplementation = getBenchmark(execDirPath, testImgPath);

		File execDir = new File(execDirPath);
		ArrayList<String> filesFromInputDir = new ArrayList<String>(
				Arrays.asList(execDir.list()));

		ArrayList<ImageFromBAM> resultImages = new ArrayList<ImageFromBAM>();

		for (int i = 0; i < filesFromInputDir.size(); i++) {
			// System.out.println(names.get(i));
			if (filesFromInputDir.get(i).endsWith(".exe")) {
				String execNamePath = execDirPath + "\\"
						+ filesFromInputDir.get(i);
				System.out.println("We should run this one: " + execNamePath);
				try {
					System.out.println(execNamePath + " " + testImgPath);

					final Process process = new ProcessBuilder(execNamePath,
							testImgPath).start();

					Thread t = new Thread() {
						public void run() {
							try {
								Thread.sleep(timerForBAM);
								process.destroy();
							} catch (InterruptedException e) {
							}
						};
					};

					InputStream is = process.getInputStream();
					InputStreamReader isr = new InputStreamReader(is);
					BufferedReader br = new BufferedReader(isr);
					String line;

					System.out.println("Output of running " + Arrays.toString(args) + " is:");

					while ((line = br.readLine()) != null) {
						System.out.println(line);
					}

					process.waitFor();
					t.interrupt();

					//get strictly image file name
					String imageName = testImgPath.substring(testImgPath
							.lastIndexOf("\\") + 1);
					imageName = imageName.replace(imageName.substring(imageName.lastIndexOf(".")), imgExtension);
					String resultImage = execDirPath + "\\dir_out\\" + filesFromInputDir.get(i) + "_" + imageName;

					ImageFromBAM img = new ImageFromBAM(resultImage);
					resultImages.add(img);

				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}

		//Get result images
		for (int i = 0; i < resultImages.size(); i++) {
			try {
				resultImages.get(i).readImage();
				resultImages.get(i).readConf();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		ImageFromBAM winner = new ImageFromBAM(execDirPath + "\\" + resultImgFN + ".tif");

		//TODO: construirea imaginii resultat
		//winner.image = resultImages.get(0).image;
		winner.image = n00bImplementation;

		winner.writeImage();

		//		String[] suffixes = ImageIO.getReaderFileSuffixes(); 
		//		for (int i = 0;i < suffixes.length; i++) { 
		//			System.out.println(suffixes[i]); 
		//		}
	}
}
