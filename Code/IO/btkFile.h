/* 
 * The Biomechanical ToolKit
 * Copyright (c) 2009-2015, Arnaud Barré
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name(s) of the copyright holders nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __btkFile_h
#define __btkFile_h

#include "btkIODevice.h"

namespace btk
{
  class FilePrivate;
  
  class BTK_IO_EXPORT File : public IODevice
  {
    BTK_DECLARE_PIMPL_ACCESSOR(File)
    
  public:
    File();
    ~File() noexcept;
    
    File(const File& ) = delete;
    File(File&& ) noexcept = delete;
    File& operator=(const File& ) = delete;
    File& operator=(File&& ) noexcept = delete;
    
    void open(const char* filename, Mode mode);
    virtual bool isOpen() const noexcept override;
    virtual void close() override;
    virtual Size peek(char* s, Size n) const override;
    virtual void read(char* s, Size n) override;
    virtual void write(const char* s, Size n) override;
    virtual void seek(Offset offset, Origin whence) override;
    virtual Position tell() const noexcept override;
    virtual bool isSequential() const noexcept override;
    
    const char* data() const noexcept;
    Size size() const noexcept;
  };
};

#endif // __btkFile_h