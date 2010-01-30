#include <string.h>

#include <utils.hh>
#include "file_browser.hh"

FileBrowser::FileBrowser(const char **exts, Font *font) : Menu(font)
{
	this->path = NULL;
	this->exts = exts;
	this->cur_path_prefix = NULL;

	if (!this->exts)
		this->exts = (const char **){ NULL };

	/* If nothing else: Set the default list */
	this->setDefaultFileList();
}

FileBrowser::~FileBrowser()
{
	this->freeFileList();
}

void FileBrowser::pushDirectory(const char *in_path)
{
	const char *cur = this->cur_path_prefix ? xstrdup(this->cur_path_prefix) : NULL;
	size_t cur_len = this->cur_path_prefix ? strlen(this->cur_path_prefix) : NULL;
	char *path_cpy = xstrdup(in_path);
	char *path = path_cpy;

	/* Weed out [ and ] from paths */
	if (*path_cpy == '[')
		path = path_cpy + 1;
	for (size_t i = 0; i < strlen(path); i++)
	{
		if (path[i] == ']')
			path[i] = '\0';
	}

	this->cur_path_prefix = (char *)xrealloc(this->cur_path_prefix,
			cur_len + 2 + strlen(path));

	if (cur != NULL)
		sprintf(this->cur_path_prefix, "%s/%s", cur, path);
	else
		strcpy(this->cur_path_prefix, path);

	free((void *)cur);
	free(path_cpy);

	this->freeFileList();
	this->file_list = get_file_list(this->cur_path_prefix, this->exts);
	if (!this->file_list)
		this->setDefaultFileList();
	this->setText(this->file_list);
}

void FileBrowser::setDirectory(const char *path)
{
	free(this->cur_path_prefix);
	this->cur_path_prefix = NULL;

	this->pushDirectory(path);
}

void FileBrowser::setDefaultFileList()
{
	this->file_list = (const char **)xmalloc(2 * sizeof(char*));
	this->file_list[0] = xstrdup("None");
}

void FileBrowser::freeFileList()
{
	if (!this->file_list)
		return;
	for (int i = 0; this->file_list[i]; i++)
		free((void*)this->file_list[i]);
	free(this->file_list);
}
