#ifndef __FILE_BROWSER_HH__
#define __FILE_BROWSER_HH__

#include "menu.hh"
#include "gui.hh"

class FileBrowser : public Menu
{
public:
	FileBrowser(const char **exts, Font *font);

	~FileBrowser();

	void pushDirectory(const char *path);

	void setDirectory(const char *path);

protected:
	void setDefaultFileList();

	void freeFileList();

	char *cur_path_prefix;
	const char *path;
	const char **file_list;
	const char **exts;
};

#endif /* __FILE_BROWSER_HH__ */
