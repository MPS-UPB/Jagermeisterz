import java.awt.Color;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;

import javax.imageio.ImageIO;
import javax.imageio.spi.IIORegistry;

public class Main {

	static String imgExtension = ".tif";
	static final Integer MIN_CONFIDENCE = 128;

	public static int getSize(BufferedImage img) throws IOException {
		//BufferedImage img = ImageIO.read(new File(path));
		int size = img.getHeight() * img.getWidth();
		return size;
	}

	// TODO: configure this method to get the .exe from project's path
	public static BufferedImage getBenchmark(String dirPath, String testImgPath) {
		String execPath = ".\\benchmark\\Noob.exe";
		try {
			System.out.println("---- getBenchmark: " + execPath + " "
					+ testImgPath);
			Process process = new ProcessBuilder(execPath, testImgPath).start();
			InputStream is = process.getInputStream();
			InputStreamReader isr = new InputStreamReader(is);
			BufferedReader br = new BufferedReader(isr);
			String line;

			while ((line = br.readLine()) != null) {
				System.out.println(line);
			}
		} catch (IOException e) {
			e.printStackTrace();
		}

		String benchmarkImgPath = execPath + "_Black_and_White.tif";
		BufferedImage img = null;
		try {
			img = ImageIO.read(new File(benchmarkImgPath));
		} catch (IOException e) {
			e.printStackTrace();
		}

		return img;
	}

	public static BufferedImage getGrayScale(BufferedImage inputImage){
		BufferedImage img = new BufferedImage(inputImage.getWidth(), inputImage.getHeight(),                BufferedImage.TYPE_BYTE_GRAY);
		Graphics g = img.getGraphics();
		g.drawImage(inputImage, 0, 0, null);
		g.dispose();
		return img;
	}

	public static void main(String[] args) throws IOException {
		// TODO Auto-generated method stub

		if (args.length != 6) {
			System.out.println("Wrong arguments!");
			return;
		}

		// Add .tif file types to ImageIO
		IIORegistry registry = IIORegistry.getDefaultInstance();
		registry.registerServiceProvider(new com.sun.media.imageioimpl.plugins.tiff.TIFFImageWriterSpi());
		registry.registerServiceProvider(new com.sun.media.imageioimpl.plugins.tiff.TIFFImageReaderSpi());

		float timeout = Float.parseFloat(args[0]);
		float iniTimeout = Float.parseFloat(args[1]);

		String execDirPath = args[2];
		String testImgPath = args[3];
		String BAMresultDirPath = args[4];

		String resultImgFN = args[5];

		BufferedImage testImg = ImageIO.read(new File(testImgPath));
		int imgSize = getSize(testImg);

		final int timerForBAM = Math.round(timeout * imgSize + iniTimeout);

		BufferedImage n00bImplementation = getBenchmark(execDirPath,
				testImgPath);

		File execDir = new File(execDirPath);
		ArrayList<String> filesFromInputDir = new ArrayList<String>(
				Arrays.asList(execDir.list()));

		ArrayList<ImageFromBAM> resultImages = new ArrayList<ImageFromBAM>();

		ArrayList<ImageProcess> imgP = new ArrayList<ImageProcess>();

		System.out.println(filesFromInputDir.size());

		for (int i = 0; i < filesFromInputDir.size(); i++) {
			// System.out.println(names.get(i));
			if (filesFromInputDir.get(i).endsWith(".exe")) {
				String execNamePath = execDirPath + "\\"
						+ filesFromInputDir.get(i);
				try {
					System.out.println("Trying to run: " + execNamePath + " " + testImgPath);

					String output = BAMresultDirPath + "\\" + 
							filesFromInputDir.get(i).substring(filesFromInputDir.get(i).lastIndexOf("\\") + 1) + 
							"_" + testImgPath.substring(testImgPath.lastIndexOf("\\") + 1);
					String conf = output + "_conf" + ".tif";
					output += ".tif";

					final Process process = new ProcessBuilder(execNamePath,
							testImgPath, output, conf).start();

					ImageProcess aux = new ImageProcess(process, output, conf);
					imgP.add(aux);		

					Thread t = new Thread() {
						public void run() {
							try {
								Thread.sleep(timerForBAM);
								//Thread.sleep(100);
								System.out.println("Time is up!");
								process.destroy();
							} catch (InterruptedException e) {
							}
						};
					};
					t.start();

					aux.t = t;

					InputStream is = process.getInputStream();
					InputStreamReader isr = new InputStreamReader(is);
					BufferedReader br = new BufferedReader(isr);
					String line;

					String[] params = {execNamePath, testImgPath, output, conf};
					System.out.println("Output of running "
							+ Arrays.toString(params) + " is:");

					while ((line = br.readLine()) != null) {
						System.out.println(line);
					}
					
					aux.p.waitFor();
					aux.t.interrupt();

					aux.exitCode = aux.p.exitValue();

					if (aux.exitCode == 0) {
						System.out.println("BAM Successfully executed!");
						String resultImage = aux.output;
						ImageFromBAM img = new ImageFromBAM(resultImage);
						resultImages.add(img);
					} else {
						System.out.println("BAM Crashed: " + aux.exitCode);
					}
				} catch (IOException e) {
					e.printStackTrace();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}

		/*
		for (ImageProcess aux : imgP) {
			try {
				aux.p.waitFor();
				aux.t.interrupt();

				aux.exitCode = aux.p.exitValue();

				if (aux.exitCode == 0) {
					String resultImage = aux.output;
					ImageFromBAM img = new ImageFromBAM(resultImage);
					resultImages.add(img);
				}
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		 */

		// Get result images
		for (int i = 0; i < resultImages.size(); i++) {
			try {
				resultImages.get(i).readImage();
				resultImages.get(i).readConf();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		System.out.println(resultImages.size());

		ImageFromBAM winner = new ImageFromBAM(n00bImplementation);
		winner.resultImagePath = resultImgFN;

		// if no result from BAMs was found, than the result will be
		// n00bImplementation

		if (resultImages.size() == 0) {
			winner.image = n00bImplementation;
		} else {
			int height = n00bImplementation.getHeight();
			int width = n00bImplementation.getWidth();

			// if there is only one result, combine noobImplementation and image
			// take the BAM implementation into account only if the confidence
			// for the pixel is bigger than a constant value
			if (resultImages.size() == 1) {
				ImageFromBAM current = resultImages.get(0);
				current.confTIF = getGrayScale(resultImages.get(0).confTIF);
				for (int j = 0; j < width; j++) {
					for (int k = 0; k < height; k++) {
						Color c = new Color(current.confTIF.getRGB(j, k), true);
						int confValueScaled = (c.getRed() + c.getGreen() + c.getBlue()) / 3;
						if (confValueScaled > MIN_CONFIDENCE) {
							winner.image.setRGB(j, k, current.image.getRGB(j, k));
						} 
					}
				}
			} else {
				for (int i = 0; i < resultImages.size(); i++) {
					ImageFromBAM current = resultImages.get(i);
					current.confTIF = getGrayScale(resultImages.get(i).confTIF);
				}
				for (int j = 0; j < width; j++) {
					for (int k = 0; k < height; k++) {

						// first, pixel is considered to have the noob value
						Color nc = new Color(n00bImplementation.getRGB(j, k));
						int ncVS = (nc.getRed() + nc.getGreen() + nc.getBlue()) / 3;
						int pixelValuesSum = ncVS;
						int imageCount = 1;
						for (int i = 0; i < resultImages.size(); i++) {

							ImageFromBAM current = resultImages.get(i);

							// if the confidence for the pixel is bigger than
							// MIN_CONFIDENCE, take the value into account
							Color c = new Color(current.confTIF.getRGB(j, k), true);
							int confValueScaled = (c.getRed() + c.getGreen() + c.getBlue()) / 3;

							if (confValueScaled > MIN_CONFIDENCE) {
								Color ci = new Color(current.image.getRGB(j, k));
								int ciValueScaled = (ci.getRed() + ci.getGreen() + ci.getBlue())/3;
								pixelValuesSum += ciValueScaled;
								imageCount++;
							}
						}
						// determine the result
						int result = pixelValuesSum / imageCount;
						Color resultColor;
						if (result <= 128) {
							result = 0;
							resultColor = new Color(0, 0, 0);
						} else {
							result = 255;
							resultColor = new Color(255, 255, 255);
						}
						winner.image.setRGB(j, k, resultColor.getRGB());
					}
				}
			}

		}
		winner.writeImage();
	}
}
