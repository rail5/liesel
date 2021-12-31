using namespace std;
using namespace Magick;

vector<Image> mayberescale(vector<Image> pamphlet, bool rescaling, double outwidth, double outheight, int quality, bool verbose, bool bookthief, int segcount, int thisseg, int numstages, bool previewonly) {

	int pagecount = pamphlet.size();
	
	size_t width = pamphlet[0].columns();
	size_t height = pamphlet[0].rows();

	if (rescaling) {
	
		size_t newwidth = width;
		size_t newheight = height;
		
		size_t widthmults[10];
		size_t heightmults[10];
		
		for (int x=0; x<10; x++) {
			widthmults[x] = (x+1)*(quality*outwidth);
			heightmults[x] = (x+1)*(quality*outheight);
		}

		int i = 0;
		while (i < 10) {
			if ((width <= widthmults[i]) || (height <= heightmults[i])) {
				newwidth = widthmults[i];
				newheight = heightmults[i];
				if (verbose == true) {
					cout << endl << "Rescaling to " << newwidth << "x" << newheight << endl;
				}
				break;
			}
					
			if ((width - widthmults[i] >= 200) || (height - heightmults[i] >= 200)) {
				i = i + 1;
			} else {
				newwidth = widthmults[i];
				newheight = heightmults[i];
				if (verbose == true) {
					cout << endl << "Rescaling to " << newwidth << "x" << newheight << endl;
				}
				break;
			}
		}
		
		Geometry newsize = Geometry(newwidth, newheight);
		newsize.aspect(true);
		for (int y=0; y<pagecount; y++) {
			if (verbose == true) {
				cout << "Rescaling page " << y+1 << "... " << flush;
			}
			
			if (bookthief == true) {
				double dprog = (double)(y + 1)/(pagecount + 1)*100;
				int prog = floor(dprog);
				progresscounter(prog, 3, numstages, segcount, thisseg);
			}
			
			pamphlet[y].resize(newsize);
			pamphlet[y].rotate(-90*previewonly); // Rotate -90*0 = 0 if not preview, -90*1 = -90 if preview
		}
		
	}
	
	return pamphlet;

}
