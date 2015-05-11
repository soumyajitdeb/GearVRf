/* Copyright 2015 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.gearvrf;

/** Frame Buffer object. */
public class GVRRenderTexture extends GVRTexture {
    /**
     * Constructs a GVRRenderTexture for a frame buffer of the specified size.
     * 
     * @param gvrContext
     *            Current gvrContext
     * @param width
     *            Width of the frame buffer.
     * @param height
     *            Height of the frame buffer.
     */
    public GVRRenderTexture(GVRContext gvrContext, int width, int height) {
        super(gvrContext, NativeRenderTexture.ctor(width, height));

        mWidth = width;
        mHeight = height;
    }

    /**
     * Constructs a GVRRenderTexture for a frame buffer of the specified size,
     * with MSAA enabled at the specified sample count.
     * 
     * @param gvrContext
     *            Current gvrContext
     * @param width
     *            Width of the frame buffer.
     * @param height
     *            Height of the frame buffer.
     * @param sampleCount
     *            MSAA sample count.
     */
    public GVRRenderTexture(GVRContext gvrContext, int width, int height,
            int sampleCount) {
        super(gvrContext, NativeRenderTexture.ctorMSAA(width, height,
                sampleCount));
        gvrContext.getRecyclableObjectProtector().addRecyclableObject(this);

        mWidth = width;
        mHeight = height;
    }

    GVRRenderTexture(GVRContext gvrContext, long ptr) {
        super(gvrContext, ptr);
    }

    /**
     * Return the width of GVRRenderTexture (FBO)
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * Return the height of GVRRenderTexture (FBO)
     */
    public int getHeight() {
        return mHeight;
    }

    @Override
    protected boolean registerWrapper() {
        // Render textures are only manipulated within the Java code; we never
        // have long-lived native references to them, and we don't want the
        // deference thread to recycle() these.
        return false;
    }

    private int mWidth, mHeight;
}

class NativeRenderTexture {
    public static native long ctor(int width, int height);

    public static native long ctorMSAA(int width, int height, int sampleCount);
}
