/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __TEXTUREREADER_H__
#define __TEXTUREREADER_H__

#include <string>
#include <vector>

namespace ghoul {

namespace opengl {
class Texture;
}

namespace io {

class TextureReaderBase;

class TextureReader {
public:
	opengl::Texture* loadTexture(const std::string& filename);
	void addReader(TextureReaderBase* reader);

	static TextureReader& ref();

protected:
	TextureReaderBase* readerForExtension(const std::string& extension);
	std::vector<TextureReaderBase*> _readers;

private:
	TextureReader();
	~TextureReader();
	TextureReader(const TextureReader&) = delete;
	TextureReader(const TextureReader&&) = delete;
	TextureReader& operator= (const TextureReader& rhs) = delete;

};

} // namespace io
} // namespace ghoul

#endif // __TEXTUREREADER_H__
