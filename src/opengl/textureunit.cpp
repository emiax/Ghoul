/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/opengl/textureunit.h>

#include <ghoul/systemcapabilities/openglcapabilitiescomponent.h>

namespace ghoul::opengl {

std::function<void*()> TextureUnit::_currentContext = []() { return nullptr; };

std::unordered_map<void*, TextureUnit::Context> TextureUnit::_contexts;

void TextureUnit::setCurrentContextFunction(std::function<void*()> f) {
    _currentContext = std::move(f);
}

TextureUnit::TextureUnitError::TextureUnitError(std::string msg)
    : RuntimeError(std::move(msg), "TextureUnit")
{}

TextureUnit::TextureUnit()
    : _number(0)
    , _glEnum(GLenum(0))
    , _assigned(false)
{
    if (_contexts.find(_currentContext()) == _contexts.end()) {
        initialize();
    }
}

TextureUnit::~TextureUnit() {
    deactivate();
}

void TextureUnit::activate() {
    if (!_assigned) {
        assignUnit();
    }
    glActiveTexture(_glEnum);
}

void TextureUnit::deactivate() {
    Context& c = _contexts[_currentContext()];
    if (_assigned) {
        _assigned = false;
        c.busyUnits.at(_number) = false;
        --c.totalActive;
    }
}

GLenum TextureUnit::glEnum() {
    if (!_assigned) {
        assignUnit();
    }
    return _glEnum;
}

GLint TextureUnit::unitNumber() {
    if (!_assigned) {
        assignUnit();
    }
    return _number;
}

TextureUnit::operator GLint() {
    return unitNumber();
}

void TextureUnit::setZeroUnit() {
    glActiveTexture(GL_TEXTURE0);
}

int TextureUnit::numberActiveUnits() {
    Context& c = _contexts[_currentContext()];
    return c.totalActive;
}

void TextureUnit::assignUnit() {
    Context& c = _contexts[_currentContext()];

    if (c.totalActive >= c.maxTexUnits) {
        throw TextureUnitError("No more texture units available");
    }

    _assigned = true;

    for (size_t i = 0; i < c.maxTexUnits; ++i) {
        if (!c.busyUnits[i]) {
            _number = static_cast<GLint>(i);
            _glEnum = GL_TEXTURE0 + _number;
            c.busyUnits[i] = true;
            ++c.totalActive;
            break;
        }
    }
}

void TextureUnit::initialize() {
    Context& c = _contexts[_currentContext()];
    if (systemcapabilities::SystemCapabilities::isInitialized()) {
        c.maxTexUnits = OpenGLCap.maxTextureUnits();
    }
    else {
        c.maxTexUnits = 8; // Reasonable default setting for OpenGL
    }
    c.busyUnits = std::vector<bool>(c.maxTexUnits, false);
}

} // namespace ghoul::opengl
