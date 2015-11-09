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

#include <ghoul/font/font.h>

#include <ghoul/misc/assert.h>
#include <ghoul/logging/logmanager.h>

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

////Replacement for Visual Studio's _vscprintf function
//#if (_MSC_VER < 1400) //if older than visual studio 2005
//static int vscprintf (const char * format, va_list pargs)
//{
//    int retval;
//    va_list argcopy;
//    va_copy(argcopy, pargs);
//    retval = vsnprintf(NULL, 0, format, argcopy);
//    va_end(argcopy);
//    return retval;
//}
//#else
//#define vscprintf(f,a) _vscprintf(f,a)
//#endif


namespace {
    const std::string _loggerCat = "Font";
    const float HighResolution = 64.f;
    const int DPI = 72;

    
    const float HighFaceResolutionFactor = 100.f;
}

namespace ghoul {
namespace fontrendering {
    
const std::string Font::AttributeAutoHinting = "AutoHinting";
const std::string Font::AttributeOutline = "Outline";
const std::string Font::AttributeOutlineThickness = "OutlineThickness";
const std::string Font::AttributeLCDFiltering = "LCDFiltering";
const std::string Font::AttributeLCDWeights = "LCDWeights";
const std::string Font::AttributeKerning = "Kerning";
const std::string Font::AttributeHeight = "Height";
const std::string Font::AttributeLinegap = "Linegap";
const std::string Font::AttributeAscender = "Ascender";
const std::string Font::AttributeDecender = "Decender";



    
    
    
Font::Glyph::Glyph(wchar_t character,
                   size_t width,
                   size_t height,
                   int offsetX,
                   int offsetY,
                   float advanceX,
                   float advanceY,
                   glm::vec2 texCoordTopLeft,
                   glm::vec2 texCoordBottomRight,
                   bool outline,
                   float outlineThickness
                   )
    : _charcode(std::move(character))
    , _width(width)
    , _height(height)
    , _offsetX(offsetX)
    , _offsetY(offsetY)
    , _advanceX(advanceX)
    , _advanceY(advanceY)
    , _topLeft(std::move(texCoordTopLeft))
    , _bottomRight(std::move(texCoordBottomRight))
    , _outline(outline)
    , _outlineThickness(outlineThickness)
{
}
    
float Font::Glyph::kerning(wchar_t character) const {
    auto it = _kerning.find(character);
    if (it != _kerning.end())
        return it->second;
    else
        return 0.f;
}
    
int Font::Glyph::offsetX() const {
    return _offsetX;
}

int Font::Glyph::offsetY() const {
    return _offsetY;
}
    
size_t Font::Glyph::width() const {
    return _width;
}
    
size_t Font::Glyph::height() const {
    return _height;
}

template <typename T>
void setValueFromDictionary(const Dictionary& attributes, const std::string& attribute, T& value) {
    if (attributes.hasKey(attribute)) {
        if (attributes.hasValue<T>(attribute))
            value = attributes.value<T>(attribute);
        else {
            LERROR("Attribute '" << attribute << "' for font was not of type " << std::string(typeid(T).name()));
        }
    }
}

    
Font::Font(std::string filename, float pointSize, opengl::TextureAtlas& atlas, const Dictionary& attributes)
    : _atlas(atlas)
    , _name(std::move(filename))
    , _pointSize(pointSize)
    , _autoHinting(true)
    , _outline(false)
    , _outlineThickness(0.15f)
    , _lcdFiltering(true)
    , _kerning(true)
    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    , _lcdWeights({{0x10, 0x40, 0x70, 0x40, 0x10}})
    , _height(0.f)
    , _linegap(0.f)
    , _ascender(0.f)
    , _decender(0.f)
//    , _underlinePosition(0.f)
//    , _underlineThickness(0.f)
{
    ghoul_assert(_pointSize > 0.f, "Need positive point size");
    ghoul_assert(!_name.empty(), "Empty file name not allowed");
    
    if (attributes.size() > 0) {
        setValueFromDictionary(attributes, AttributeAutoHinting, _autoHinting);
        setValueFromDictionary(attributes, AttributeOutline, _outline);
        setValueFromDictionary(attributes, AttributeOutlineThickness, _outlineThickness);
        setValueFromDictionary(attributes, AttributeLCDFiltering, _lcdFiltering);
        setValueFromDictionary(attributes, AttributeLCDWeights, _lcdWeights);
        setValueFromDictionary(attributes, AttributeKerning, _kerning);
        setValueFromDictionary(attributes, AttributeHeight, _height);
        setValueFromDictionary(attributes, AttributeLinegap, _linegap);
        setValueFromDictionary(attributes, AttributeAscender, _ascender);
        setValueFromDictionary(attributes, AttributeDecender, _decender);
        
        static const std::array<std::string, 10> AllAttributes = {{
            AttributeAutoHinting,
            AttributeOutline,
            AttributeOutlineThickness,
            AttributeLCDFiltering,
            AttributeLCDWeights,
            AttributeKerning,
            AttributeHeight,
            AttributeLinegap,
            AttributeAscender,
            AttributeDecender
        }};
        
        for (const std::string& key : attributes.keys()) {
            auto it = std::find(AllAttributes.begin(), AllAttributes.end(), key);
            if (it == AllAttributes.end())
                LWARNING("Unknown key '" << key << "' for fonts");
        }
    }
}
    
Font::~Font() {
        
}
    
bool Font::operator==(const Font& rhs) {
    return (
        (_name == rhs._name) &&
        (_pointSize == rhs._pointSize) &&
        (_glyphs == rhs._glyphs) &&
        (&_atlas == &rhs._atlas) &&
        (_autoHinting == rhs._autoHinting) &&
        (_outline == rhs._outline) &&
        (_outlineThickness == rhs._outlineThickness) &&
        (_lcdFiltering == rhs._lcdFiltering) &&
        (_kerning == rhs._kerning) &&
        (_lcdWeights == rhs._lcdWeights) &&
        (_height = rhs._height) &&
        (_linegap == rhs._linegap) &&
        (_ascender == rhs._ascender) &&
        (_decender == rhs._decender)
    );
}
    
bool Font::initialize() {
    FT_Library library;
    FT_Face face;

    // Get font metrics at high resolution
    bool success = loadFace(_pointSize * HighFaceResolutionFactor, library, face);
    if (!success)
        return false;
    
    
    FT_Size_Metrics metrics = face->size->metrics;
    _ascender = (metrics.ascender >> 6) / 100.f;
    _decender = (metrics.descender >> 6) / 100.f;
    _height = (metrics.height >> 6) / 100.f;
    _linegap = _height - _ascender + _decender;
    
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    
    /* -1 is a special glyph */
    glyph(-1);
    
    return true;
}
    
Font::Glyph* Font::glyph(wchar_t character) {
    // Check if charcode has been already loaded
    for (size_t i = 0; i < _glyphs.size(); ++i) {
        Glyph* glyph = _glyphs[i];
        
        bool correctCharacter = (glyph->_charcode == character);
        bool isSpecialCharacter = (character == static_cast<wchar_t>(-1));
        bool correctOutlineType = glyph->_outline == _outline;
        bool correctOutlineThickness = glyph->outlineThickness() == _outlineThickness;
        

        if (correctCharacter && (isSpecialCharacter || (correctOutlineType &&
             correctOutlineThickness)))
        {
            return glyph;
        }
        
    }
    
    // charcode -1 is special: it is used for line drawing (overline, underline,
    // strikethrough) and background.
    if (character == static_cast<wchar_t>(-1)) {
        size_t width = _atlas.width();
        size_t height = _atlas.height();
        glm::ivec4 region = _atlas.allocateRegion(5, 5);
        if (region.x < 0)
            return nullptr;

        // The last *4 for the depth is not a danger here as _atlas.setRegion only
        // extracts as much data as is needed for the used texture atlas
        std::array<unsigned char, 4*4*4> data;
        data.fill(std::numeric_limits<unsigned char>::max());
        
        _atlas.setRegion(region.x, region.y, 4, 4, data.data(), 0);
        
        Glyph* glyph = new Glyph(static_cast<wchar_t>(-1));
        glyph->_topLeft = glm::vec2(
            (region.x+2)/static_cast<float>(width),
            (region.y+2)/static_cast<float>(height)
        );
        glyph->_bottomRight = glm::vec2(
            (region.x+3)/static_cast<float>(width),
            (region.y+3)/static_cast<float>(height)
        );

        _glyphs.push_back(glyph);

        return glyph;
    }
    
    // Glyph has not been already loaded
    size_t nGlyphNotLoaded = loadGlyphs({character});
    if (nGlyphNotLoaded == 0)
        return _glyphs.back();
    else {
        LERROR(nGlyphNotLoaded << " glyphs could not be loaded");
        return nullptr;
    }
}
    
std::string Font::name() const {
    return _name;
}
    
float Font::pointSize() const {
    return _pointSize;
}

bool Font::autoHinting() const {
    return _autoHinting;
}

bool Font::outline() const {
    return _outline;
}

float Font::outlineThickness() const {
    return _outlineThickness;
}

bool Font::lcdFiltering() const {
    return lcdFiltering();
}

bool Font::kerning() const {
    return _kerning;
}

std::array<unsigned char, 5> Font::lcdWeights() const {
    return _lcdWeights;
}

float Font::height() const {
    return _height;
}

float Font::linegap() const {
    return _linegap;
}

float Font::ascender() const {
    return _ascender;
}

float Font::decender() const {
    return _decender;
}
    
    
opengl::TextureAtlas& Font::atlas() {
    return _atlas;
}
    
size_t Font::loadGlyphs(const std::vector<wchar_t>& glyphs) {
    size_t missed = 0;
    
    size_t width  = _atlas.width();
    size_t height = _atlas.height();
    size_t depth  = _atlas.depth();
    
    FT_Library library;
    FT_Face face;
    bool success = loadFace(_pointSize, library, face);
    if (!success)
        return glyphs.size();

    // Search through the loaded glyphs to avoid duplicates
    for (size_t i = 0; i < glyphs.size(); ++i) {
        bool foundGlyph = false;
        /* Check if charcode has been already loaded */
        for (size_t j = 0; j < _glyphs.size(); ++j ) {
            const Glyph * const glyph = _glyphs[j];
            // If charcode is -1, we don't care about outline type or thickness
            // if( (glyph->charcode == charcodes[i])) {
            bool correctCharcode = glyph->_charcode == glyphs[i];
            bool specialGlyph = glyphs[i] == static_cast<wchar_t>(-1);
            bool correctOutline =
                (glyph->_outline == _outline) &&
                (glyph->outlineThickness() == _outlineThickness);
            
            if (correctCharcode && (specialGlyph || correctOutline)) {
                foundGlyph = true;
                break;
            }
        }
        
        if (foundGlyph)
            continue;
        
        FT_Int32 flags = 0;
        int ft_glyph_top = 0;
        int ft_glyph_left = 0;
        FT_UInt glyph_index = FT_Get_Char_Index(face, glyphs[i]);
        // WARNING: We use texture-atlas depth to guess if user wants
        //          LCD subpixel rendering
        
        if (_outline)
            flags |= FT_LOAD_NO_BITMAP;
        else
            flags |= FT_LOAD_RENDER;
        
        if (!_autoHinting)
            flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
        else
            flags |= FT_LOAD_FORCE_AUTOHINT;
        
        if (depth == 3) {
            FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
            flags |= FT_LOAD_TARGET_LCD;
            if (_lcdFiltering)
                FT_Library_SetLcdFilterWeights(library, _lcdWeights.data());
        }
        
        FT_Error error = FT_Load_Glyph(face, glyph_index, flags);
        if (error) {
            LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            return glyphs.size() - i;
        }
        
        
        FT_GlyphSlot slot;
        FT_Bitmap ft_bitmap;
        FT_Glyph ft_glyph;
        if (!_outline) {
            slot            = face->glyph;
            ft_bitmap       = slot->bitmap;
            ft_glyph_top    = slot->bitmap_top;
            ft_glyph_left   = slot->bitmap_left;
        }
        else {
            FT_Stroker stroker;
            FT_BitmapGlyph ft_bitmap_glyph;
            FT_Error error = FT_Stroker_New(library, &stroker);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return 0;
            }
            
            FT_Stroker_Set(stroker,
                           static_cast<int>(_outlineThickness * HighResolution),
                           FT_STROKER_LINECAP_ROUND,
                           FT_STROKER_LINEJOIN_ROUND,
                           0);
            error = FT_Get_Glyph(face->glyph, &ft_glyph);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return 0;
            }
            
            error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
            if (error) {
                LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return 0;
            }
            
            if (depth == 1) {
                error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                if (error) {
                    LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                    FT_Done_Face(face);
                    FT_Stroker_Done(stroker);
                    FT_Done_FreeType(library);
                    return 0;
                }
            }
            else {
                error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
                if (error) {
                    LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
                    FT_Done_Face(face);
                    FT_Stroker_Done(stroker);
                    FT_Done_FreeType(library);
                    return 0;
                }
            }
            ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
            ft_bitmap       = ft_bitmap_glyph->bitmap;
            ft_glyph_top    = ft_bitmap_glyph->top;
            ft_glyph_left   = ft_bitmap_glyph->left;
            FT_Stroker_Done(stroker);
        }
        
        
        // We want each glyph to be separated by at least one black pixel
        // (for example for shader used in demo-subpixel.c)
        size_t w = ft_bitmap.width/depth + 1;
        size_t h = ft_bitmap.rows + 1;
        glm::ivec4 region = _atlas.allocateRegion(w, h);
        if (region.x < 0) {
            missed++;
            LERROR("Texture atlas is full");
            continue;
        }
        w = w - 1;
        h = h - 1;
        size_t x = region.x;
        size_t y = region.y;
        _atlas.setRegion(x, y, w, h, ft_bitmap.buffer, ft_bitmap.pitch);
        
        
        // Discard hinting to get advance
        FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
        
        Glyph* glyph = new Glyph(
            glyphs[i],
            w,
            h,
            ft_glyph_left,
            ft_glyph_top,
            face->glyph->advance.x / HighResolution,
            face->glyph->advance.y / HighResolution,
            glm::vec2(
                x/static_cast<float>(width),
                y/static_cast<float>(height)
            ),
            glm::vec2(
                (x + w)/static_cast<float>(width),
                (y + h)/static_cast<float>(height)
            ),
            _outline,
            _outlineThickness
        );
        
        _glyphs.push_back(glyph);
        
        if (_outline)
            FT_Done_Glyph(ft_glyph);
    }
    
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    _atlas.upload();
    generateKerning();
    return missed;
}
    
void Font::generateKerning() {
    FT_Library library;
    FT_Face face;
    
    bool success = loadFace(_pointSize, library, face);
    if (!success)
        return;
    
    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special backgroudn glyph */
    for (size_t i = 1; i < _glyphs.size(); ++i) {
        Glyph* glyph = _glyphs[i];
        FT_UInt glyphIndex = FT_Get_Char_Index(face, glyph->_charcode);
        glyph->_kerning.clear();
        
        for (size_t j = 1; j < _glyphs.size(); ++j) {
            Glyph* prevGlyph = _glyphs[j];
            FT_UInt prevIndex = FT_Get_Char_Index(face, prevGlyph->_charcode);
            FT_Vector kerning;
            FT_Get_Kerning(face, prevIndex, glyphIndex, FT_KERNING_UNFITTED, &kerning);
            if (kerning.x != 0) {
                glyph->_kerning[prevGlyph->_charcode] = kerning.x / (HighResolution*HighResolution);
            }
        }
    }
    
    FT_Done_Face( face );
    FT_Done_FreeType( library );
}
    
bool Font::loadFace(float size, FT_Library& library, FT_Face& face) {
    FT_Matrix matrix = {
        (int)((1.0/HighResolution) * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((1.0)      * 0x10000L)};
    
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
        return 0;
    }
    
    /* Load face */
    error = FT_New_Face(library, _name.c_str(), 0, &face);
    
    if (error) {
        LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
        FT_Done_FreeType(library);
        return 0;
    }
    
    /* Select charmap */
    error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    if (error) {
        LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return 0;
    }
    
    /* Set char size */
    error = FT_Set_Char_Size(face, (int)(size * HighResolution), 0, DPI * HighResolution, DPI);
    
    if (error) {
        LERROR("FT_Error: " << FT_Errors[error].code << " (" << FT_Errors[error].message << ")");
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return 0;
    }
    
    /* Set transform matrix */
    FT_Set_Transform(face, &matrix, NULL);
    
    return 1;
}

    
} // namespace fontrendering
} // namespace ghoul
