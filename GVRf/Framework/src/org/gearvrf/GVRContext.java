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

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

import org.gearvrf.GVRAndroidResource.BitmapTextureCallback;
import org.gearvrf.GVRAndroidResource.CompressedTextureCallback;
import org.gearvrf.GVRAndroidResource.MeshCallback;
import org.gearvrf.animation.GVRAnimation;
import org.gearvrf.animation.GVRAnimationEngine;
import org.gearvrf.asynchronous.GVRAsynchronousResourceLoader;
import org.gearvrf.asynchronous.GVRCompressedTexture;
import org.gearvrf.asynchronous.GVRCompressedTextureLoader;
import org.gearvrf.periodic.GVRPeriodicEngine;
import org.gearvrf.utility.Log;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.KeyEvent;

/**
 * Like the Android {@link Context} class, {@code GVRContext} provides core
 * services, and global information about an application environment.
 * 
 * Use {@code GVRContext} to {@linkplain #createQuad(float, float) create} and
 * {@linkplain #loadMesh(String) load} GL meshes, Android
 * {@linkplain #loadBitmap(String) bitmaps}, and
 * {@linkplain #loadTexture(String) GL textures.} {@code GVRContext} also holds
 * the {@linkplain GVRScene main scene} and miscellaneous information like
 * {@linkplain #getFrameTime() the frame time.}
 */
public abstract class GVRContext {
    private static final String TAG = Log.tag(GVRContext.class);

    private final Context mContext;

    /*
     * Fields and constants
     */

    // Priorities constants, for asynchronous loading

    /**
     * GVRF can't use every {@code int} as a priority - it needs some sentinel
     * values. It will probably never need anywhere near this many, but raising
     * the number of reserved values narrows the 'dynamic range' available to
     * apps mapping some internal score to the {@link #LOWEST_PRIORITY} to
     * {@link #HIGHEST_PRIORITY} range, and might change app behavior in subtle
     * ways that seem best avoided.
     * 
     * @since 1.6.1
     */
    public static final int RESERVED_PRIORITIES = 1024;

    /**
     * GVRF can't use every {@code int} as a priority - it needs some sentinel
     * values. A simple approach to generating priorities is to score resources
     * from 0 to 1, and then map that to the range {@link #LOWEST_PRIORITY} to
     * {@link #HIGHEST_PRIORITY}.
     * 
     * @since 1.6.1
     */
    public static final int LOWEST_PRIORITY = Integer.MIN_VALUE
            + RESERVED_PRIORITIES;

    /**
     * GVRF can't use every {@code int} as a priority - it needs some sentinel
     * values. A simple approach to generating priorities is to score resources
     * from 0 to 1, and then map that to the range {@link #LOWEST_PRIORITY} to
     * {@link #HIGHEST_PRIORITY}.
     * 
     * @since 1.6.1
     */
    public static final int HIGHEST_PRIORITY = Integer.MAX_VALUE;

    /**
     * The priority used by
     * {@link #loadBitmapTexture(GVRAndroidResource.BitmapTextureCallback, GVRAndroidResource)}
     * and
     * {@link #loadMesh(GVRAndroidResource.MeshCallback, GVRAndroidResource)}
     * 
     * @since 1.6.1
     */
    public static final int DEFAULT_PRIORITY = 0;

    /**
     * The ID of the GLthread. We use this ID to prevent non-GL thread from
     * calling GL functions.
     * 
     * @since 1.6.5
     */
    protected long mGLThreadID;

    /*
     * Methods
     */

    GVRContext(Context context) {
        mContext = context.getApplicationContext();
    }

    /**
     * Get the Android {@link Context}, which provides access to system services
     * and to your application's resources. This is <em>not</em> your
     * {@link GVRActivity} implementation, but rather the
     * {@linkplain Activity#getApplicationContext() application context,} which
     * is usually an {@link Application}.
     * 
     * @return An Android {@code Context}
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Loads a file as a {@link GVRMesh}.
     * 
     * Note that this method can be quite slow; we recommend never calling it
     * from the GL thread. The asynchronous version
     * {@link #loadMesh(GVRAndroidResource.MeshCallback, GVRAndroidResource)} is
     * better because it moves most of the work to a background thread, doing as
     * little as possible on the GL thread.
     * 
     * @param androidResource
     *            Basically, a stream containing a 3D model. The
     *            {@link GVRAndroidResource} class has six constructors to
     *            handle a wide variety of Android resource types. Taking a
     *            {@code GVRAndroidResource} here eliminates six overloads.
     * @return The file as a GL mesh.
     * 
     * @since 1.6.2
     */
    public GVRMesh loadMesh(GVRAndroidResource androidResource) {
        GVRAssimpImporter assimpImporter = GVRImporter.readFileFromResources(
                this, androidResource);
        return assimpImporter.getMesh(0);
    }

    /**
     * Loads a mesh file, asynchronously, at a default priority.
     * 
     * This method and the
     * {@linkplain #loadMesh(GVRAndroidResource.MeshCallback, GVRAndroidResource, int)
     * overload that takes a priority} are generally going to be your best
     * choices for loading {@link GVRMesh} resources: mesh loading can take
     * hundreds - and even thousands - of milliseconds, and so should not be
     * done on the GL thread in either {@link GVRScript#onInit(GVRContext)
     * onInit()} or {@link GVRScript#onStep() onStep()}.
     * 
     * <p>
     * The asynchronous methods improve throughput in three ways. First, by
     * doing all the work on a background thread, then delivering the loaded
     * mesh to the GL thread on a {@link #runOnGlThread(Runnable)
     * runOnGlThread()} callback. Second, they use a throttler to avoid
     * overloading the system and/or running out of memory. Third, they do
     * 'request consolidation' - if you issue any requests for a particular file
     * while there is still a pending request, the file will only be read once,
     * and each callback will get the same {@link GVRMesh}.
     * 
     * @param callback
     *            App supplied callback, with three different methods.
     *            <ul>
     *            <li>Before loading, GVRF may call
     *            {@link GVRAndroidResource.MeshCallback#stillWanted(GVRAndroidResource)
     *            stillWanted()} (on a background thread) to give you a chance
     *            to abort a 'stale' load.
     * 
     *            <li>Successful loads will call
     *            {@link GVRAndroidResource.Callback#loaded(GVRHybridObject, GVRAndroidResource)
     *            loaded()} on the GL thread.
     * 
     *            <li>Any errors will call
     *            {@link GVRAndroidResource.MeshCallback#failed(Throwable, GVRAndroidResource)
     *            failed(),} with no promises about threading.
     *            </ul>
     * @param androidResource
     *            Basically, a stream containing a 3D model. The
     *            {@link GVRAndroidResource} class has six constructors to
     *            handle a wide variety of Android resource types. Taking a
     *            {@code GVRAndroidResource} here eliminates six overloads.
     * 
     * @throws IllegalArgumentException
     *             If either parameter is {@code null}
     * 
     * @since 1.6.2
     */
    public void loadMesh(MeshCallback callback,
            GVRAndroidResource androidResource) throws IllegalArgumentException {
        loadMesh(callback, androidResource, DEFAULT_PRIORITY);
    }

    /**
     * Loads a mesh file, asynchronously, at an explicit priority.
     * 
     * This method and the
     * {@linkplain #loadMesh(GVRAndroidResource.MeshCallback, GVRAndroidResource)
     * overload that supplies a default priority} are generally going to be your
     * best choices for loading {@link GVRMesh} resources: mesh loading can take
     * hundreds - and even thousands - of milliseconds, and so should not be
     * done on the GL thread in either {@link GVRScript#onInit(GVRContext)
     * onInit()} or {@link GVRScript#onStep() onStep()}.
     * 
     * <p>
     * The asynchronous methods improve throughput in three ways. First, by
     * doing all the work on a background thread, then delivering the loaded
     * mesh to the GL thread on a {@link #runOnGlThread(Runnable)
     * runOnGlThread()} callback. Second, they use a throttler to avoid
     * overloading the system and/or running out of memory. Third, they do
     * 'request consolidation' - if you issue any requests for a particular file
     * while there is still a pending request, the file will only be read once,
     * and each callback will get the same {@link GVRMesh}.
     * 
     * @param callback
     *            App supplied callback, with three different methods.
     *            <ul>
     *            <li>Before loading, GVRF may call
     *            {@link GVRAndroidResource.MeshCallback#stillWanted(GVRAndroidResource)
     *            stillWanted()} (on a background thread) to give you a chance
     *            to abort a 'stale' load.
     * 
     *            <li>Successful loads will call
     *            {@link GVRAndroidResource.Callback#loaded(GVRHybridObject, GVRAndroidResource)
     *            loaded()} on the GL thread.
     * 
     *            <li>Any errors will call
     *            {@link GVRAndroidResource.MeshCallback#failed(Throwable, GVRAndroidResource)
     *            failed(),} with no promises about threading.
     *            </ul>
     * @param resource
     *            Basically, a stream containing a 3D model. The
     *            {@link GVRAndroidResource} class has six constructors to
     *            handle a wide variety of Android resource types. Taking a
     *            {@code GVRAndroidResource} here eliminates six overloads.
     * @param priority
     *            This request's priority. Please see the notes on asynchronous
     *            priorities in the <a href="package-summary.html#async">package
     *            description</a>.
     * 
     * @throws IllegalArgumentException
     *             If either {@code callback} or {@code resource} is
     *             {@code null}, or if {@code priority} is out of range.
     * 
     * @since 1.6.2
     */
    public void loadMesh(MeshCallback callback, GVRAndroidResource resource,
            int priority) throws IllegalArgumentException {
        GVRAsynchronousResourceLoader.loadMesh(this, callback, resource,
                priority);
    }

    /**
     * Creates a quad consisting of two triangles, with the specified width and
     * height.
     * 
     * @param width
     *            the quad's width
     * @param height
     *            the quad's height
     * @return A 2D, rectangular mesh with four vertices and two triangles
     */
    public GVRMesh createQuad(float width, float height) {
        GVRMesh mesh = new GVRMesh(this);

        float[] vertices = { width * -0.5f, height * 0.5f, 0.0f, width * -0.5f,
                height * -0.5f, 0.0f, width * 0.5f, height * 0.5f, 0.0f,
                width * 0.5f, height * -0.5f, 0.0f };
        mesh.setVertices(vertices);

        float[] texCoords = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
        mesh.setTexCoords(texCoords);

        char[] triangles = { 0, 1, 2, 1, 3, 2 };
        mesh.setTriangles(triangles);

        return mesh;
    }

    /**
     * Loads a 3D model file as a {@code GVRScene}.
     * 
     * This method and the
     * {@linkplain #loadModel(String)} are both capable of loading 
     * a 3D model file with all its components. The {@linkplain #loadScene(String)}
     * method is the best choices for loading a 3D model resources as a {@code GVRScene}
     * and the {@linkplain #loadModel(String)} is the best choice
     * when trying to load 3D model as a {@code GVRSceneObject} 
     * 
     * @param fileName
     *            The name of a file placed in asset or res folder. The asset
     *            and res directory may contain an arbitrarily complex tree of
     *            sub directories; the file name can specify any location in or
     *            under the assets and res directory.
     */
    public void loadScene(String fileName)
    {
        GVRAssimpImporter assimpImporter = null;
        try {
            assimpImporter = GVRImporter.readFileFromResources(this, new GVRAndroidResource(this, fileName));
        } catch (IOException e) {
            e.printStackTrace();
        }
        //Default bitmap for objects whose texture are not present or there is some texture error
        Bitmap defaultBitmap = GVRAsynchronousResourceLoader.decodeStream((new GVRAndroidResource(this, R.drawable.__default_bitmap__)).getStream(), false);
        
        //Scene
        GVRScene completeScene = assimpImporter.loadScene(defaultBitmap, this);
        
        // Sets the main scene with the model
        this.setMainScene(completeScene);
    }

    /**
     * Loads a 3D model file.as a {@code GVRSceneObject}.
     * 
     * This method and the {@linkplain #loadScene(String)} are both capable of loading 
     * a 3D model file with all its components. {@linkplain #loadModel(String)} 
     * methods is the best choice when trying to load 3D model resource as a 
     * {@code GVRSceneObject} and the {@linkplain #loadScene(String)} method is the 
     * best choices for loading a 3D model resources as a part of the {@code GVRScene}.
     * 
     * @param fileName
     *            The name of a file placed in asset or res folder. The asset
     *            and res directory may contain an arbitrarily complex tree of
     *            sub directories; the file name can specify any location in or
     *            under the assets and res directory.
     * 
     * @return The 3D model as a {@code GVRSceneObject}.
     */
    public GVRSceneObject loadModel(String fileName)
    {
        GVRAssimpImporter assimpImporter = null; 
        try {
            assimpImporter = GVRImporter.readFileFromResources(this, new GVRAndroidResource(this, fileName));
        } catch (IOException e) {
            e.printStackTrace();
        }
        //Default bitmap for objects whose texture are not present or there is some texture error
        Bitmap defaultBitmap = GVRAsynchronousResourceLoader.decodeStream((new GVRAndroidResource(this, R.drawable.__default_bitmap__)).getStream(), false);
        
        //Scene
        GVRScene completeScene = assimpImporter.loadScene(defaultBitmap, this);

        GVRSceneObject sceneAsSceneObject = new GVRSceneObject(this);
        GVRSceneObject[] wholeSceneObjects = completeScene.getWholeSceneObjects();
        for (GVRSceneObject sceneObject:wholeSceneObjects)
        {
            sceneAsSceneObject.addChildObject(sceneObject);
        }
        return sceneAsSceneObject;
    }

    /**
     * Loads file placed in the assets folder, as a {@link Bitmap}.
     * 
     * <p>
     * Note that this method may take hundreds of milliseconds to return: unless
     * the bitmap is quite tiny, you probably don't want to call this directly
     * from your {@link GVRScript#onStep() onStep()} callback as that is called
     * once per frame, and a long call will cause you to miss frames.
     * 
     * <p>
     * Note also that this method does no scaling, and will return a full-size
     * {@link Bitmap}. Loading (say) an unscaled photograph may abort your app:
     * Use pre-scaled images, or {@link BitmapFactory} methods which give you
     * more control over the image size.
     * 
     * @param fileName
     *            The name of a file, relative to the assets directory. The
     *            assets directory may contain an arbitrarily complex tree of
     *            subdirectories; the file name can specify any location in or
     *            under the assets directory.
     * @return The file as a bitmap, or {@code null} if file path does not exist
     *         or the file can not be decoded into a Bitmap.
     */
    public Bitmap loadBitmap(String fileName) {
        if (fileName == null) {
            throw new IllegalArgumentException("File name should not be null.");
        }
        InputStream stream = null;
        Bitmap bitmap = null;
        try {
            try {
                stream = mContext.getAssets().open(fileName);
                return bitmap = BitmapFactory.decodeStream(stream);
            } finally {
                if (stream != null) {
                    stream.close();
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            // Don't discard a valid Bitmap because of an IO error closing the
            // file!
            return bitmap;
        }
    }

    /**
     * Loads file placed in the assets or res folder, as a {@link Bitmap}.
     * 
     * @param fileName
     *            The name of a file placed in asset or res folder. The asset
     *            and res directory may contain an arbitrarily complex tree of
     *            sub directories; the file name can specify any location in or
     *            under the assets and res directory.
     * @return The file as a bitmap, or a default bitmap if file path does not exist
     *         or {@code null} if the file can not be decoded into a Bitmap.
     */

    public Bitmap loadBitmapFromResOrAsset(String fileName) {
        GVRAndroidResource resource = null;
        try {
            String tempFileName = "";
            try {
            	if (fileName.indexOf(".") > 0)
            	{
            		tempFileName = fileName.substring(0, fileName.indexOf("."));
            	}
            	else
            	{
            		tempFileName = fileName;
            	}
                Class res = R.drawable.class;
                Field field = res.getField(tempFileName);
                int drawableId = field.getInt(null);
                resource = new GVRAndroidResource(this, drawableId);
            }
            catch (Exception e) {
                // Failure to get drawable id.
            	resource = new GVRAndroidResource(this, fileName);
            }            
    
        } catch (IOException e) {
            e.printStackTrace();
            return GVRAsynchronousResourceLoader.decodeStream((new GVRAndroidResource(this, R.drawable.__default_bitmap__)).getStream(), false);
        }
        assertGLThread();
        Bitmap bitmap = GVRAsynchronousResourceLoader.decodeStream(resource.getStream(), false);
        resource.closeStream();
        return bitmap;
    }

    /**
     * Loads file placed in the assets folder, as a {@link GVRBitmapTexture}.
     * 
     * <p>
     * Note that this method may take hundreds of milliseconds to return: unless
     * the texture is quite tiny, you probably don't want to call this directly
     * from your {@link GVRScript#onStep() onStep()} callback as that is called
     * once per frame, and a long call will cause you to miss frames. For large
     * images, you should use either
     * {@link #loadBitmapTexture(GVRAndroidResource.BitmapTextureCallback, GVRAndroidResource)
     * loadBitmapTexture()} (faster) or
     * {@link #loadCompressedTexture(GVRAndroidResource.CompressedTextureCallback, GVRAndroidResource)}
     * (fastest <em>and</em> least memory pressure).
     * 
     * <p>
     * Note also that this method does no scaling, and will return a full-size
     * {@link Bitmap}. Loading (say) an unscaled photograph may abort your app:
     * Use
     * <ul>
     * <li>Pre-scaled images
     * <li>{@link BitmapFactory} methods which give you more control over the
     * image size, or
     * <li>
     * {@link #loadTexture(GVRAndroidResource)} or
     * {@link #loadBitmapTexture(GVRAndroidResource.BitmapTextureCallback, GVRAndroidResource)}
     * which automatically scale large images to fit the GPU's restrictions and
     * to avoid {@linkplain OutOfMemoryError out of memory errors.}
     * </ul>
     * 
     * @param fileName
     *            The name of a file, relative to the assets directory. The
     *            assets directory may contain an arbitrarily complex tree of
     *            sub-directories; the file name can specify any location in or
     *            under the assets directory.
     * @return The file as a texture, or {@code null} if file path does not
     *         exist or the file can not be decoded into a Bitmap.
     * 
     * @deprecated We will remove this blocking function during Q3 of 2015. We
     *             suggest that you switch to
     *             {@link #loadTexture(GVRAndroidResource)}
     * 
     */
    public GVRBitmapTexture loadTexture(String fileName) {

        assertGLThread();

        if (fileName.endsWith(".png")) { // load png directly to texture
            return new GVRBitmapTexture(this, fileName);
        }

        Bitmap bitmap = loadBitmap(fileName);
        return bitmap == null ? null : new GVRBitmapTexture(this, bitmap);
    }

    /**
     * Loads file placed in the assets folder, as a {@link GVRBitmapTexture}.
     * 
     * <p>
     * Note that this method may take hundreds of milliseconds to return: unless
     * the texture is quite tiny, you probably don't want to call this directly
     * from your {@link GVRScript#onStep() onStep()} callback as that is called
     * once per frame, and a long call will cause you to miss frames. For large
     * images, you should use either
     * {@link #loadBitmapTexture(GVRAndroidResource.BitmapTextureCallback, GVRAndroidResource)
     * loadBitmapTexture()} (faster) or
     * {@link #loadCompressedTexture(GVRAndroidResource.CompressedTextureCallback, GVRAndroidResource)}
     * (fastest <em>and</em> least memory pressure).
     * 
     * <p>
     * This method automatically scales large images to fit the GPU's
     * restrictions and to avoid {@linkplain OutOfMemoryError out of memory
     * errors.} </ul>
     * 
     * @param androidResource
     *            Basically, a stream containing a bitmap texture. The
     *            {@link GVRAndroidResource} class has six constructors to
     *            handle a wide variety of Android resource types. Taking a
     *            {@code GVRAndroidResource} here eliminates six overloads.
     * @return The file as a texture, or {@code null} if the file can not be
     *         decoded into a Bitmap.
     * 
     * @since 1.6.5
     */
    public GVRBitmapTexture loadTexture(GVRAndroidResource resource) {

        assertGLThread();

        Bitmap bitmap = GVRAsynchronousResourceLoader.decodeStream(
                resource.getStream(), false);
        resource.closeStream();
        return bitmap == null ? null : new GVRBitmapTexture(this, bitmap);
    }

    /**
     * Throws an exception if the current thread is not a GL thread.
     * 
     * @since 1.6.5
     * 
     */
    private void assertGLThread() {

        if (Thread.currentThread().getId() != mGLThreadID) {
            throw new RuntimeException(
                    "Should not run GL functions from a non-GL thread!");
        }

    }

    /**
     * Load a bitmap, asynchronously, with a default priority.
     * 
     * Because it is asynchronous, this method <em>is</em> a bit harder to use
     * than {@link #loadTexture(String)}, but it moves a large amount of work
     * (in {@link BitmapFactory#decodeStream(InputStream)} from the GL thread to
     * a background thread. Since you <em>can</em> create a
     * {@link GVRSceneObject} without a mesh and texture - and set them later -
     * using the asynchronous API can improve startup speed and/or reduce frame
     * misses (where an {@link GVRScript#onStep() onStep()} takes too long).
     * This API may also use less RAM than {@link #loadTexture(String)}.
     * 
     * <p>
     * This API will 'consolidate' requests: If you request a texture like
     * {@code R.raw.wood_grain} and then - before it has loaded - issue another
     * request for {@code R.raw.wood_grain}, GVRF will only read the bitmap file
     * once; only create a single {@link GVRTexture}; and then call both
     * callbacks, passing each the same texture.
     * 
     * <p>
     * Please be aware that {@link BitmapFactory#decodeStream(InputStream)} is a
     * comparatively expensive operation: it can take hundreds of milliseconds
     * and use several megabytes of temporary RAM. GVRF includes a throttler to
     * keep the total load manageable - but
     * {@link #loadCompressedTexture(GVRAndroidResource.CompressedTextureCallback, GVRAndroidResource)}
     * is <em>much</em> faster and lighter-weight: that API simply loads the
     * compressed texture into a small amount RAM (which doesn't take very long)
     * and does some simple parsing to figure out the parameters to pass
     * {@code glCompressedTexImage2D()}. The GL hardware does the decoding much
     * faster than Android's {@link BitmapFactory}!
     * 
     * <p>
     * TODO Take a boolean parameter that controls mipmap generation?
     * 
     * @since 1.6.1
     * 
     * @param callback
     *            Before loading, GVRF may call
     *            {@link GVRAndroidResource.BitmapTextureCallback#stillWanted(GVRAndroidResource)
     *            stillWanted()} several times (on a background thread) to give
     *            you a chance to abort a 'stale' load.
     * 
     *            Successful loads will call
     *            {@link GVRAndroidResource.Callback#loaded(GVRHybridObject, GVRAndroidResource)
     *            loaded()} on the GL thread;
     * 
     *            any errors will call
     *            {@link GVRAndroidResource.BitmapTextureCallback#failed(Throwable, GVRAndroidResource)
     *            failed()}, with no promises about threading.
     * 
     *            <p>
     *            This method uses a throttler to avoid overloading the system.
     *            If the throttler has threads available, it will run this
     *            request immediately. Otherwise, it will enqueue the request,
     *            and call
     *            {@link GVRAndroidResource.BitmapTextureCallback#stillWanted(GVRAndroidResource)
     *            stillWanted()} at least once (on a background thread) to give
     *            you a chance to abort a 'stale' load.
     * @param resource
     *            Basically, a stream containing a bitmapped image. The
     *            {@link GVRAndroidResource} class has six constructors to
     *            handle a wide variety of Android resource types. Taking a
     *            {@code GVRAndroidResource} here eliminates six overloads.
     */
    public void loadBitmapTexture(BitmapTextureCallback callback,
            GVRAndroidResource resource) {
        loadBitmapTexture(callback, resource, DEFAULT_PRIORITY);
    }

    /**
     * Load a bitmap, asynchronously, with an explicit priority.
     * 
     * Because it is asynchronous, this method <em>is</em> a bit harder to use
     * than {@link #loadTexture(String)}, but it moves a large amount of work
     * (in {@link BitmapFactory#decodeStream(InputStream)} from the GL thread to
     * a background thread. Since you <em>can</em> create a
     * {@link GVRSceneObject} without a mesh and texture - and set them later -
     * using the asynchronous API can improve startup speed and/or reduce frame
     * misses, where an {@link GVRScript#onStep() onStep()} takes too long.
     * 
     * <p>
     * This API will 'consolidate' requests: If you request a texture like
     * {@code R.raw.wood_grain} and then - before it has loaded - issue another
     * request for {@code R.raw.wood_grain}, GVRF will only read the bitmap file
     * once; only create a single {@link GVRTexture}; and then call both
     * callbacks, passing each the same texture.
     * 
     * <p>
     * Please be aware that {@link BitmapFactory#decodeStream(InputStream)} is a
     * comparatively expensive operation: it can take hundreds of milliseconds
     * and use several megabytes of temporary RAM. GVRF includes a throttler to
     * keep the total load manageable - but
     * {@link #loadCompressedTexture(GVRAndroidResource.CompressedTextureCallback, GVRAndroidResource)}
     * is <em>much</em> faster and lighter-weight: that API simply loads the
     * compressed texture into a small amount RAM (which doesn't take very long)
     * and does some simple parsing to figure out the parameters to pass
     * {@code glCompressedTexImage2D()}. The GL hardware does the decoding much
     * faster than Android's {@link BitmapFactory}!
     * 
     * @since 1.6.1
     * 
     * @param callback
     *            Before loading, GVRF may call
     *            {@link GVRAndroidResource.BitmapTextureCallback#stillWanted(GVRAndroidResource)
     *            stillWanted()} several times (on a background thread) to give
     *            you a chance to abort a 'stale' load.
     * 
     *            Successful loads will call
     *            {@link GVRAndroidResource.Callback#loaded(GVRHybridObject, GVRAndroidResource)
     *            loaded()} on the GL thread;
     * 
     *            any errors will call
     *            {@link GVRAndroidResource.BitmapTextureCallback#failed(Throwable, GVRAndroidResource)
     *            failed()}, with no promises about threading.
     * 
     *            <p>
     *            This method uses a throttler to avoid overloading the system.
     *            If the throttler has threads available, it will run this
     *            request immediately. Otherwise, it will enqueue the request,
     *            and call
     *            {@link GVRAndroidResource.BitmapTextureCallback#stillWanted(GVRAndroidResource)
     *            stillWanted()} at least once (on a background thread) to give
     *            you a chance to abort a 'stale' load.
     * @param resource
     *            Basically, a stream containing a bitmapped image. The
     *            {@link GVRAndroidResource} class has six constructors to
     *            handle a wide variety of Android resource types. Taking a
     *            {@code GVRAndroidResource} here eliminates six overloads.
     * @param priority
     *            This request's priority. Please see the notes on asynchronous
     *            priorities in the <a href="package-summary.html#async">package
     *            description</a>.
     * 
     * @throws IllegalArgumentException
     *             If {@code priority} {@literal <} {@link #LOWEST_PRIORITY} or
     *             {@literal >} {@link #HIGHEST_PRIORITY}, or either of the
     *             other parameters is {@code null}
     */
    public void loadBitmapTexture(BitmapTextureCallback callback,
            GVRAndroidResource resource, int priority)
            throws IllegalArgumentException {
        GVRAsynchronousResourceLoader.loadBitmapTexture(this, callback,
                resource, priority);
    }

    /**
     * Load a compressed texture, asynchronously.
     * 
     * GVRF currently supports ASTC, ETC2, and KTX formats: applications can add
     * new formats by implementing {@link GVRCompressedTextureLoader}.
     * 
     * <p>
     * This method uses the fastest possible rendering. To specify higher
     * quality (but slower) rendering, you can use the
     * {@link #loadCompressedTexture(GVRAndroidResource.CompressedTextureCallback, GVRAndroidResource, int)}
     * overload.
     * 
     * @since 1.6.1
     * 
     * @param callback
     *            Successful loads will call
     *            {@link GVRAndroidResource.Callback#loaded(GVRHybridObject, GVRAndroidResource)
     *            loaded()} on the GL thread; any errors will call
     *            {@link GVRAndroidResource.CompressedTextureCallback#failed(Throwable, GVRAndroidResource)
     *            failed()}, with no promises about threading.
     * @param resource
     *            Basically, a stream containing a compressed texture. The
     *            {@link GVRAndroidResource} class has six constructors to
     *            handle a wide variety of Android resource types. Taking a
     *            {@code GVRAndroidResource} here eliminates six overloads.
     */
    public void loadCompressedTexture(CompressedTextureCallback callback,
            GVRAndroidResource resource) {
        GVRAsynchronousResourceLoader.loadCompressedTexture(this, callback,
                resource);
    }

    /**
     * Load a compressed texture, asynchronously.
     * 
     * GVRF currently supports ASTC, ETC2, and KTX formats: applications can add
     * new formats by implementing {@link GVRCompressedTextureLoader}.
     * 
     * @since 1.6.1
     * 
     * @param callback
     *            Successful loads will call
     *            {@link GVRAndroidResource.Callback#loaded(GVRHybridObject, GVRAndroidResource)}
     *            on the GL thread; any errors will call
     *            {@link GVRAndroidResource.CompressedTextureCallback#failed(Throwable, GVRAndroidResource)}
     *            , with no promises about threading.
     * @param resource
     *            Basically, a stream containing a compressed texture. The
     *            {@link GVRAndroidResource} class has six constructors to
     *            handle a wide variety of Android resource types. Taking a
     *            {@code GVRAndroidResource} here eliminates six overloads.
     * @param quality
     *            Speed/quality tradeoff: should be one of
     *            {@link GVRCompressedTexture#SPEED},
     *            {@link GVRCompressedTexture#BALANCED}, or
     *            {@link GVRCompressedTexture#QUALITY}, but other values are
     *            'clamped' to one of the recognized values.
     */
    public void loadCompressedTexture(CompressedTextureCallback callback,
            GVRAndroidResource resource, int quality) {
        GVRAsynchronousResourceLoader.loadCompressedTexture(this, callback,
                resource, quality);
    }

    /**
     * Get the current {@link GVRScene}, which contains the scene graph (a
     * hierarchy of {@linkplain GVRSceneObject scene objects}) and the
     * {@linkplain GVRCameraRig camera rig}
     * 
     * @return A {@link GVRScene} instance, containing scene and camera
     *         information
     */
    public abstract GVRScene getMainScene();

    /** Set the current {@link GVRScene} */
    public abstract void setMainScene(GVRScene scene);

    /**
     * Returns a {@link GVRScene} that you can populate before passing to
     * {@link #setMainScene(GVRScene)}.
     * 
     * Implementation maintains a single element buffer, initialized to
     * {@code null}. When this method is called, creates a new scene if the
     * buffer is {@code null}, then returns the buffered scene. If this buffered
     * scene is passed to {@link #setMainScene(GVRScene)}, the buffer is reset
     * to {@code null}.
     * 
     * <p>
     * One use of this is to build your scene graph while the splash screen is
     * visible. If you have called {@linkplain #getNextMainScene()} (so that the
     * next-main-scene buffer is non-{@code null} when the splash screen is
     * closed) GVRF will automatically switch to the 'pending' main-scene; if
     * the buffer is {@code null}, GVRF will simply remove the splash screen
     * from the main scene object.
     * 
     * @since 1.6.4
     */
    public GVRScene getNextMainScene() {
        return getNextMainScene(null);
    }

    /**
     * Returns a {@link GVRScene} that you can populate before passing to
     * {@link #setMainScene(GVRScene)}.
     * 
     * Implementation maintains a single element buffer, initialized to
     * {@code null}. When this method is called, creates a new scene if the
     * buffer is {@code null}, then returns the buffered scene. If this buffered
     * scene is passed to {@link #setMainScene(GVRScene)}, the buffer is reset
     * to {@code null}.
     * 
     * <p>
     * One use of this is to build your scene graph while the splash screen is
     * visible. If you have called {@linkplain #getNextMainScene()} (so that the
     * next-main-scene buffer is non-{@code null} when the splash screen is
     * closed) GVRF will automatically switch to the 'pending' main-scene; if
     * the buffer is {@code null}, GVRF will simply remove the splash screen
     * from the main scene object.
     * 
     * @param onSwitchMainScene
     *            Optional (may be {@code null}) {@code Runnable}, called when
     *            this {@link GVRScene} becomes the new main scene, whether
     *            {@linkplain #setMainScene(GVRScene) explicitly} or implicitly
     *            (as, for example, when the splash screen closes). This
     *            callback lets apps do things like start animations when their
     *            scene becomes visible, instead of in
     *            {@link GVRScript#onInit(GVRContext) onInit()} when the scene
     *            objects may be hidden by the splash screen.
     * 
     * @since 1.6.4
     */
    public abstract GVRScene getNextMainScene(Runnable onSwitchMainScene);

    /**
     * Is the key pressed?
     * 
     * @param keyCode
     *            An Android {@linkplain KeyEvent#KEYCODE_0 key code}
     */
    public abstract boolean isKeyDown(int keyCode);

    /**
     * The interval between this frame and the previous frame, in seconds: a
     * rough gauge of Frames Per Second.
     */
    public abstract float getFrameTime();

    /**
     * Enqueues a callback to be run in the GL thread.
     * 
     * This is how you take data generated on a background thread (or the main
     * (GUI) thread) and pass it to the coprocessor, using calls that must be
     * made from the GL thread (aka the "GL context"). The callback queue is
     * processed before any registered
     * {@linkplain #registerDrawFrameListener(GVRDrawFrameListener) frame
     * listeners}.
     * 
     * @param runnable
     *            A bit of code that must run on the GL thread
     */
    public abstract void runOnGlThread(Runnable runnable);

    /**
     * Subscribes a {@link GVRDrawFrameListener}.
     * 
     * Each frame listener is called, once per frame, after any pending
     * {@linkplain #runOnGlThread(Runnable) GL callbacks} and before
     * {@link GVRScript#onStep()}.
     * 
     * @param frameListener
     *            A callback that will fire once per frame, until it is
     *            {@linkplain #unregisterDrawFrameListener(GVRDrawFrameListener)
     *            unregistered}
     */
    public abstract void registerDrawFrameListener(
            GVRDrawFrameListener frameListener);

    /**
     * Remove a previously-subscribed {@link GVRDrawFrameListener}.
     * 
     * @param frameListener
     *            An instance of a {@link GVRDrawFrameListener} implementation.
     *            Unsubscribing a listener which is not actually subscribed will
     *            not throw an exception.
     */
    public abstract void unregisterDrawFrameListener(
            GVRDrawFrameListener frameListener);

    /**
     * The {@linkplain GVRMaterialShaderManager object shader manager}
     * singleton.
     * 
     * Use the shader manager to define custom GL object shaders, which are used
     * to render a scene object's surface.
     * 
     * @return The {@linkplain GVRMaterialShaderManager shader manager}
     *         singleton.
     */
    public GVRMaterialShaderManager getMaterialShaderManager() {
        return getRenderBundle().getMaterialShaderManager();
    }

    /**
     * The {@linkplain GVRPostEffectShaderManager scene shader manager}
     * singleton.
     * 
     * Use the shader manager to define custom GL scene shaders, which can be
     * inserted into the rendering pipeline to apply image processing effects to
     * the rendered scene graph. In classic GL programming, this is often
     * referred to as a "post effect."
     * 
     * @return The {@linkplain GVRPostEffectShaderManager post effect shader
     *         manager} singleton.
     */
    public GVRPostEffectShaderManager getPostEffectShaderManager() {
        return getRenderBundle().getPostEffectShaderManager();
    }

    /**
     * The {@linkplain GVRAnimationEngine animation engine} singleton.
     * 
     * Use the animation engine to start and stop {@linkplain GVRAnimation
     * animations}.
     * 
     * @return The {@linkplain GVRAnimationEngine animation engine} singleton.
     */
    public GVRAnimationEngine getAnimationEngine() {
        return GVRAnimationEngine.getInstance(this);
    }

    /**
     * The {@linkplain GVRPeriodicEngine periodic engine} singleton.
     * 
     * Use the periodic engine to schedule {@linkplain Runnable runnables} to
     * run on the GL thread at a future time.
     * 
     * @return The {@linkplain GVRPeriodicEngine periodic engine} singleton.
     */
    public GVRPeriodicEngine getPeriodicEngine() {
        return GVRPeriodicEngine.getInstance(this);
    }

    /**
     * Register a method that is called every time GVRF creates a new
     * {@link GVRContext}.
     * 
     * Android apps aren't mapped 1:1 to Linux processes; the system may keep a
     * process loaded even after normal complete shutdown, and call Android
     * lifecycle methods to reinitialize it. This causes problems for (in
     * particular) lazy-created singletons that are tied to a particular
     * {@code GVRContext}. This method lets you register a handler that will be
     * called on restart, which can reset your {@code static} variables to the
     * compiled-in start state.
     * 
     * <p>
     * For example,
     * 
     * <pre>
     * 
     * static YourSingletonClass sInstance;
     * static {
     *     GVRContext.addResetOnRestartHandler(new Runnable() {
     * 
     *         &#064;Override
     *         public void run() {
     *             sInstance = null;
     *         }
     *     });
     * }
     * 
     * </pre>
     * 
     * <p>
     * GVRF will force an Android garbage collection after running any handlers,
     * which will free any remaining native objects from the previous run.
     * 
     * @param handler
     *            Callback to run on restart.
     */
    public synchronized static void addResetOnRestartHandler(Runnable handler) {
        sHandlers.add(handler);
    }

    protected synchronized static void resetOnRestart() {
        for (Runnable handler : sHandlers) {
            Log.d(TAG, "Running on-restart handler %s", handler);
            handler.run();
        }

        // We've probably just nulled-out a bunch of references, but many GVRF
        // apps do relatively little Java memory allocation, so it may actually
        // be a longish while before the recyclable references go stale.
        System.gc();

        // We do NOT want to clear sHandlers - the static initializers won't be
        // run again, even if the new run does recreate singletons.
    }

    private static final List<Runnable> sHandlers = new ArrayList<Runnable>();

    abstract GVRReferenceQueue getReferenceQueue();

    abstract GVRRenderBundle getRenderBundle();

    abstract GVRRecyclableObjectProtector getRecyclableObjectProtector();
}
