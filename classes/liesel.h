using namespace std;

namespace Liesel {

	class Book {

		private:
			poppler::document* document;
			
			Hans::Document lhdoc;
			
			struct props {
				bool grayscale = false;
				bool alterthreshold = false;
				bool cropflag = false;
				bool dividepages = false;
				bool widenflag = false;
				bool automargin = false;
				
				int quality = 100;
				double threshold = ProxyMaxRGB / 2;
				vector<int> cropvalues = {0, 0, 0, 0};
				int widenby = 0;
				int maxmargin = 0;
			};
			
			struct job {
				bool landscapeflip = false;
				bool previewonly = false;
				bool rescaling = false;
				bool linear_output = false;
				
				bool segmented = false;
				
				int segsize = 1;
				int finalsegsize = 1;
				int segcount = 1;
				
				int thisseg = 1;
				int startfrom = 1;
				int endat = 1;
				
				int minsize = 4;
				
				double rescale_width = 0.0;
				double rescale_height = 0.0;
				
				string outfilename = "";
			};
			
		public:
			bool loaded = false;
			/**********
				True if the document has been successfully loaded
			**********/
			
			int pagecount = 0;
			/**********
				The total number of pages in the PDF, set by count_pages()
			**********/
			
			vector<int> selectedpages;
			
			struct props properties;
			/**********
				Struct detailing certain changes requested by the user
		
				properties.grayscale : boolean = are we converting to grayscale?
			
				properties.alterthreshold : boolean = are we doing the 'color threshold'?
			
				properties.cropflag : boolean = are we cropping?
				
				properties.dividepages : boolean = are we dividing each page into 2?
				
				properties.widenflag : boolean = are we widening the center margin (the space between left/right-hand pages)?
				
				properties.automargin : boolean = are we *progressively* widening the center margin? (wider towards the center of the booklet)
				
				properties.quality : integer = PPI / quality
				
				properties.threshold : double = the color threshold
				
				properties.cropvalues : vector of integers = how much to crop {left, right, top, bottom}
				
				properties.widenby : integer = if we're widening the center margin, by how much?
				
				properties.maxmargin : integer = if we're *progressively* widening the center margin, what's the maximum? (widenby here acts as the minimum)
			**********/
			
			struct job printjob;
			/**********
				Struct detailing the current print job
				
				printjob.landscapeflip : boolean = should we flip every other page 180 degrees? (for long-edge [standard] duplex printing)
				
				printjob.previewonly : boolean = are we exporting a preview jpeg rather than processing a full PDF?
				
				printjob.rescaling : boolean = did the user request a transform/rescale?
				
				printjob.linear_output : boolean = did the user request linear output? (skip making a booklet, just apply requested changes)
				
				
				printjob.segmented : boolean = are we doing segmented printing? (-s option)
				
				
				printjob.segsize : integer = the number of pages per segment, if doing segmented printing
				
				printjob.finalsegsize : integer = the number of pages for the final segment (can be different to ordinary segments)
				
				printjob.segcount : integer = the total number of segments to be printed
				
				printjob.numstages : integer = the number of stages (2 normally (load + combine pages), 3 if we're doing a rescale)
				
				printjob.thisseg : integer = the current segment
				
				printjob.startfrom : integer = the index, within the selectedpages vector, from which this job is starting
					
					ie, if we're doing the following pages
						selectedpages = {8, 9, 10, 27, 28, 1, 2, 3, 99}
					and:
						printjob.startfrom = 4
					then we're starting with page #28:
						selectedpages = {8, 9, 10, 27, (((28))), 1, 2, 3, 99}
					being the 4th (counting from 0) element of the vector
					
				printjob.endat : integer = the index, within the selectedpages vector, at which this job is ending
				
				printjob.minsize : integer = absolute minimum segment size (if the final segment size is less than this number, the pages get tacked on to the penultimate segment, and the segment count is decremented by 1)
				
				
				printjob.rescale_width : double = (if rescaling) output width
				
				printjob.rescale_height : double = (if rescaling) output height
				
				printjob.outfilename : string = the base output filename (if segmented, will replace '.pdf' with -part1.pdf, -part2.pdf, etc)
			**********/
	
			bool load_document(const string &input, bool pdfstdin = false, bool speak = true);
			/**********
				Loads the input PDF for further processing
			
				The "input" string can be either:
					1. A file path to a PDF on disk
						in which case, pdfstdin = false,
					2. The raw contents of the PDF
						in which case, pdfstdin = true
					
				If speak == true, will write errors to stderr
			
				Returns true on successful load, false on failure
			**********/
		
			void count_pages(bool verbose);
			/**********
				Counts the PDF's pages, and updates the "pagecount" int attribute
				If verbose == true, will write to stdout as well
			**********/
		
			bool set_pages(bool rangeflag, string rangevalue = "");
			/**********
				Sets the pages we'll be processing from the PDF to the vector<int> selectedpages variable
				
				If rangeflag == false, rangevalue is ignored and we load all pages
				
				If rangeflag == true, rangevalue is processed and validated
					rangevalue example:
					1-100,105-115,10-3,4,5,6
					
				Only run AFTER load_document & count_pages, as this has to validate rangevalue against the document's pagecount
				
				Returns true on success, false on failure
			**********/
			
			void calculate_segments();
			/**********
				If user specifies -s for segmented printing, calculates the total number of segments, if any blanks are needed, etc
			**********/
			
			void run_job(bool verbose, bool bookthief, bool pdfstdout);
			/**********
				Runs the actual print job (load_pages, make_booklet, rescale, and write to file/stdout)
			**********/
			
			void display_progress(int progress, int stage);
			/**********
				Prints current progress to STDOUT
				
				int progress = how far along we are in the current operation
				int stage = which operation? (1 = load_pages, 2 = make_booklet, 3 = rescale)
			**********/
			
			vector<Magick::Image> pages;
			/**********
				Vector of the actual rendered pages
			**********/
			
			void load_pages(bool verbose, bool bookthief);
			/**********
				Loads the pages into the pages vector
			**********/
			
			vector<Magick::Image> booklet;
			/**********
				Vector of the finished booklet
			**********/
			
			void make_booklet(bool verbose, bool bookthief);
			/**********
				Combines the pages from the "pages" vector into a booklet, in the "booklet" vector
			**********/
			
			void rescale(bool verbose, bool bookthief);
			/**********
				Rescales output to user-specified arbitrary size (e.g, 8.5x11)
			**********/
	};
	
	Book load_book(const string &input, bool pdfstdin = false, bool speak = true);
	/**********
		Initializes Book with load_document()
		ie,
			Liesel::Book ourbook = Liesel::load_book("file.pdf");
		or another example,
			Liesel::Book ourbook = Liesel::load_book(stdin, true, false);
	**********/
}
