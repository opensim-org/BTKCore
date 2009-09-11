/* 
 * The Biomechanical ToolKit
 * Copyright (c) 2009, Arnaud Barré
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

#ifndef __btkMetaDataInfo_p_h
#define __btkMetaDataInfo_p_h

#include "btkConvert.h"

#include <vector>

namespace btk
{
  typedef enum {String=-1, Int8=1, Int16=2, Int=3, Float=4, UInt8=5, UInt16=6, UInt=7, Double=8} Convert2Format;

  // Voidify
  template <typename T>
  inline void* Voidify_p(const T& source)
  {
    T* target = new T;
    *target = source;
    return static_cast<void*>(target);
  };


  template <typename T>
  inline void Voidify_p(const std::vector<T>& source, std::vector<void*>& target)
  {
    target.resize(source.size(), 0);
    size_t inc = 0;
    while (inc < static_cast<int>(source.size()))
    {
      target[inc] = Voidify_p(source[inc]);
      ++inc;  
    }
  };

  template <typename T>
  inline void Voidify_p(int num, const std::vector<T>& source, std::vector<void*>& target)
  {
    std::vector<T> values = source;
    values.resize(num);
    Voidify_p(values, target);
  };

  // Devoidify
  template <typename T>
  inline T Devoidify_p(void* source)
  {
    return *(static_cast<T*>(source));
  };

  template <typename T>
  inline T Devoidify_p(const std::vector<void*>& source, int idx)
  {
    if (idx >= static_cast<int>(source.size()))
    {
      btkErrorMacro("Index out of range. Default value returned.");
      return T();
    }
    return Devoidify_p<T>(source[idx]);
  };

  template <typename T>
  inline void Devoidify_p(const std::vector<void*>& source, std::vector<T>& target)
  {
    target.resize(source.size());
    for (int i = 0 ; i < static_cast<int>(target.size()) ; ++i)
      target[i] = Devoidify_p<T>(source[i]);
  };

  template <typename T>
  inline void Nullify_p(const std::vector<void*>& source, std::vector<T>& target)
  {
    target.resize(source.size());
    for (int i = 0 ; i < static_cast<int>(target.size()) ; ++i)
      target[i] = T();
  };

  template <typename S, typename T>
  inline T Numerify_p(const std::vector<void*>& source, int idx)
  {
    if (idx >= static_cast<int>(source.size()))
    {
      btkErrorMacro("Index out of range. Default value returned.");
      return T();
    }
    return static_cast<T>(Devoidify_p<S>(source[idx]));
  };

  template <typename S, typename T>
  inline void Numerify_p(const std::vector<void*>& source, std::vector<T>& target)
  {
    target.resize(source.size());
    for (int i = 0 ; i < static_cast<int>(target.size()) ; ++i)
      target[i] = static_cast<T>(Devoidify_p<S>(source[i]));
  };

  template <typename T>
  inline const std::string Stringify_p(const std::vector<void*>& source, int idx)
  {
    if (idx >= static_cast<int>(source.size()))
    {
      btkErrorMacro("Index out of range. Default value returned.");
      return "";
    }
    return ToString(Devoidify_p<T>(source[idx]));
  };

  template <typename T>
  inline void Stringify_p(const std::vector<void*>& source, std::vector<std::string>& target)
  {
    target.resize(source.size());
    for (int i = 0 ; i < static_cast<int>(target.size()) ; ++i)
      target[i] = ToString(Devoidify_p<T>(source[i]));
  };

  // Single
  template <typename T>
  inline T Devoidify_p(MetaDataInfo::Format f, const std::vector<void*>& source, int idx, Convert2Format of)
  {
    if (static_cast<int>(f) == static_cast<int>(of))
      return Devoidify_p<T>(source, idx);
    else
    {
      switch(f)
      {
        case MetaDataInfo::Byte:
          return Numerify_p<int8_t, T>(source, idx);
          break;
        case MetaDataInfo::Integer:
          return Numerify_p<int16_t, T>(source, idx);
          break;
        case MetaDataInfo::Real:
          return Numerify_p<float, T>(source, idx);
          break;
        case MetaDataInfo::Char:
          return T();
          break;
        default:
          return T();
      }
    }
  };

  template <>
  inline std::string Devoidify_p<std::string>(MetaDataInfo::Format f, const std::vector<void*>& source, int idx, Convert2Format of)
  {
    switch(f)
    {
      case MetaDataInfo::Byte:
        return Stringify_p<int8_t>(source, idx);
        break;
      case MetaDataInfo::Integer:
        return Stringify_p<int16_t>(source, idx);
        break;
      case MetaDataInfo::Real:
        return Stringify_p<float>(source, idx);
        break;
      case MetaDataInfo::Char:
        return Stringify_p<std::string>(source, idx);
        break;
      default:
        return "";

    }
  };

  // Vector
  template <typename T>
  inline void Devoidify_p(MetaDataInfo::Format f, const std::vector<void*>& source, std::vector<T>& target, Convert2Format of)
  {
    if (static_cast<int>(f) == static_cast<int>(of))
      Devoidify_p(source, target);
    else
    {
      switch(f)
      {
        case MetaDataInfo::Byte:
          Numerify_p<int8_t>(source, target);
          break;
        case MetaDataInfo::Integer:
          Numerify_p<int16_t>(source, target);
          break;
        case MetaDataInfo::Real:
          Numerify_p<float>(source, target);
          break;
        case MetaDataInfo::Char:
          Nullify_p(source, target);
          break;
      }
    }
  };

  template <>
  inline void Devoidify_p<std::string>(MetaDataInfo::Format f, const std::vector<void*>& source, std::vector<std::string>& target, Convert2Format of)
  {
    switch(f)
    {
      case MetaDataInfo::Byte:
        Stringify_p<int8_t>(source, target);
        break;
      case MetaDataInfo::Integer:
        Stringify_p<int16_t>(source, target);
        break;
      case MetaDataInfo::Real:
        Stringify_p<float>(source, target);
        break;
      case MetaDataInfo::Char:
        Stringify_p<std::string>(source, target);
        break;
    }
  };

  template <typename T>
  inline std::vector<T> Devoidify_p(MetaDataInfo::Format f, const std::vector<void*>& source, Convert2Format of)
  {
    std::vector<T> target;
    Devoidify_p(f, source, target, of);
    return target;
  };

  // Delete / Erase / Clear
  template <typename T>
  inline void Delete_p(void* target)
  {
    delete static_cast<T*>(target);
  };

  template <typename T>
  inline void Delete_p(std::vector<void*>::iterator start, std::vector<void*>::iterator end)
  {
    for (std::vector<void*>::iterator it = start ; it != end ; ++it)
      delete static_cast<T*>(*it);
  };

  inline void Delete_p(MetaDataInfo::Format f, std::vector<void*>::iterator start, std::vector<void*>::iterator end)
  {
    switch(f)
    {
      case MetaDataInfo::Byte:
        Delete_p<int8_t>(start, end);
        break;
      case MetaDataInfo::Integer:
        Delete_p<int16_t>(start, end);
        break;
      case MetaDataInfo::Real:
        Delete_p<float>(start, end);
        break;
      case MetaDataInfo::Char:
        Delete_p<std::string>(start, end);
        break;
    }
  };

  inline void Erase_p(MetaDataInfo::Format f, std::vector<void*>& target, std::vector<void*>::iterator start, std::vector<void*>::iterator end)
  {
    Delete_p(f, start, end);
    target.erase(start, end);
  };

  inline void Clear_p(MetaDataInfo::Format f, std::vector<void*>& target)
  {
    std::vector<void*>::iterator start = target.begin();
    std::vector<void*>::iterator end = target.end();
    Delete_p(f, start, end);
    target.clear();
  };

  // Copy
  template <typename T>
  inline void* Copy_p(void* source)
  {
    T* target = new T;
    *target = *(static_cast<T*>(source));
    return static_cast<void*>(target);
  };

  template <typename T>
  inline void Copy_p(const std::vector<void*>& source, std::vector<void*>& target)
  {
    target.resize(source.size(), 0);
    for (int i = 0 ; i < static_cast<int>(target.size()) ; ++i)
      target[i] = Copy_p<T>(source[i]);
  };

  inline void Copy_p(MetaDataInfo::Format f, const std::vector<void*>& source, std::vector<void*>& target)
  {
    switch(f)
    {
      case MetaDataInfo::Byte:
        Delete_p<int8_t>(target.begin(), target.end());
        Copy_p<int8_t>(source, target);
        break;
      case MetaDataInfo::Integer:
        Delete_p<int16_t>(target.begin(), target.end());
        Copy_p<int16_t>(source, target);
        break;
      case MetaDataInfo::Real:
        Delete_p<float>(target.begin(), target.end());
        Copy_p<float>(source, target);
        break;
      case MetaDataInfo::Char:
        Delete_p<std::string>(target.begin(), target.end());
        Copy_p<std::string>(source, target);
        break;
    }
  };

  // Insert
  template <typename T>
  inline void Insert_p(std::vector<void*>& target, std::vector<void*>::iterator loc, int num, const T& val)
  {
    for (int i = 0 ; i < num ; ++i)
      loc = target.insert(loc, Voidify_p(val));
  };

  inline void Insert_p(MetaDataInfo::Format f, std::vector<void*>& target, std::vector<void*>::iterator loc, int num)
  {
    switch(f)
    {
      case MetaDataInfo::Byte:
        Insert_p<int8_t>(target, loc, num, 0);
        break;
      case MetaDataInfo::Integer:
        Insert_p<int16_t>(target, loc, num, 0);
        break;
      case MetaDataInfo::Real:
        Insert_p<float>(target, loc, num, 0.0);
        break;
      case MetaDataInfo::Char:
        Insert_p<std::string>(target, loc, num, " ");
        break;
    }
  };

  // Resize
  template <typename T>
  inline void Resize_p(std::vector<void*>& target, int num, const T& val)
  {
    if (num == target.size())
      return;
    if (num > static_cast<int>(target.size()))
      Insert_p(target, target.end(), num, val);
    else
    {
      std::vector<void*>::iterator start = target.begin();
      std::advance(start, num);
      Delete_p<T>(start, target.end());
      target.erase(start, target.end());
    }
  };

  inline void Resize_p(MetaDataInfo::Format f, std::vector<void*>& target, int num)
  {
    switch(f)
    {
      case MetaDataInfo::Byte:
        Resize_p<int8_t>(target, num, 0);
        break;
      case MetaDataInfo::Integer:
        Resize_p<int16_t>(target, num, 0);
        break;
      case MetaDataInfo::Real:
        Resize_p<float>(target, num, 0.0);
        break;
      case MetaDataInfo::Char:
        Resize_p<std::string>(target, num, " ");
        break;
    }
  };

};
#endif // __btkMetaDataInfo_p_h