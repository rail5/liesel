/**************
"Hans" is a bare-bones C++ wrapper around libHaru
**************/

using namespace std;

namespace Hans {

	class Document {
	
		private:
			HPDF_Error_Handler temp_handler;
			void* voidpointer;
			HPDF_Doc doc = HPDF_New(temp_handler, voidpointer);
		public:
		
			int width = 612;
			int height = 792;
		
			void initialize();
		
			void page_from_image_blob(Magick::Blob image);
			
			void lhdoc_from_image_vector(vector<Magick::Image> &booklet);
			
			void write_to_file(string outputfilename);
			
			void write_to_stdout();
	
	};

}
