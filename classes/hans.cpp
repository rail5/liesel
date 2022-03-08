#include "hans.h"

using namespace std;

void Hans::Document::initialize() {
	doc = HPDF_New(temp_handler, voidpointer);
	HPDF_SetCompressionMode(doc, HPDF_COMP_ALL);
}

void Hans::Document::page_from_image_blob(Magick::Blob image) {
	HPDF_Page newpage = HPDF_AddPage(doc);

	HPDF_Page_SetWidth(newpage, width);
	HPDF_Page_SetHeight(newpage, height);

	HPDF_Image loaded = HPDF_LoadJpegImageFromMem(doc, (const unsigned char*)image.data(), image.length());

	HPDF_Page_DrawImage(newpage, loaded, 0, 0, width, height);
}

void Hans::Document::lhdoc_from_image_vector(vector<Magick::Image> &booklet) {
	initialize();
	long unsigned int totalpagecount = booklet.size();
	for (long unsigned int i=0;i<totalpagecount;i++) {
		Magick::Blob pageblob;
		booklet[0].magick("JPEG");
		booklet[0].write(&pageblob);

		booklet.erase(booklet.begin());

		page_from_image_blob(pageblob);
	}
}

void Hans::Document::write_to_file(string outputfilename) {
	HPDF_SaveToFile(doc, outputfilename.c_str());
}

void Hans::Document::write_to_stdout() {
	HPDF_SaveToStream(doc);

	unsigned int streamsize = HPDF_GetStreamSize(doc);

	unsigned char buf[streamsize];

	HPDF_ReadFromStream(doc, buf, &streamsize);

	for (unsigned int i=0;i<streamsize;i++) {
		printf("%c", buf[i]);
	}
}
