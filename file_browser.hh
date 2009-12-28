#ifndef __FILE_BROWSER_HH__
#define __FILE_BROWSER_HH__

#include "menu.hh"
#include "gui.hh"

class FileBrowser : public Menu
{
public:
	FileBrowser(const char **exts, Font *font) : Menu(font)
	{
		this->path = NULL;
		this->exts = exts;

		/* If nothing else: Set the default list */
		this->setDefaultFileList();
	}

	~FileBrowser()
	{
		this->freeFileList();
	}

	void setDirectory(const char *path)
	{
		this->freeFileList();
		this->file_list = get_file_list(path, this->exts);
		if (!this->file_list)
			this->setDefaultFileList();
		this->setText(this->file_list);
	}

protected:
	void setDefaultFileList()
	{
		this->file_list = (const char **)xmalloc(2 * sizeof(char*));
		this->file_list[0] = xstrdup("None");
	}

	void freeFileList()
	{
		if (!this->file_list)
			return;
		for (int i = 0; this->file_list[i]; i++)
			free((void*)this->file_list[i]);
		free(this->file_list);
	}

	const char *path;
	const char **file_list;
	const char **exts;
};

#endif /* __FILE_BROWSER_HH__ */
