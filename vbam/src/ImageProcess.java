
public class ImageProcess {

	Process p;
	Thread t;
	int exitCode;
	
	String output;
	String conf;
	
	public ImageProcess(Process p, String output, String conf) {
		this.p = p;
		this.output = output;
		this.conf = conf;
	}
	
}
