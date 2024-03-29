#ifndef _QCAR_TEXTURE_H_
#define _QCAR_TEXTURE_H_

#include <jni.h>

/// A utility class for textures.
class Texture
{
public:

  Texture();
  ~Texture();

  /// Returns the width of the texture.
  unsigned int
  getWidth() const;

  /// Returns the height of the texture.
  unsigned int
  getHeight() const;

  /// Create a texture from a jni object:
  static Texture*
  create(JNIEnv* env, jobject textureObject);

  /// The width of the texture.
  unsigned int mWidth;

  /// The height of the texture.
  unsigned int mHeight;

  /// The number of channels of the texture.
  unsigned int mChannelCount;

  /// The pointer to the raw texture data.
  unsigned char* mData;

  /// The ID of the texture
  unsigned int mTextureID;
};

#endif
