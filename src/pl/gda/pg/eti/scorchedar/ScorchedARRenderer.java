package pl.gda.pg.eti.scorchedar;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import com.qualcomm.QCAR.QCAR;

public class ScorchedARRenderer extends Activity implements GLSurfaceView.Renderer  {
	public boolean mIsActive = false;

	/** initialize the renderer */
	public native void initRendering();

	/** update the renderer. */
	public native void updateRendering(int width, int height);

	/** Called when the surface is created or recreated. */
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		DebugLog.LOGD("GLRenderer::onSurfaceCreated");

		initRendering();

		// Call QCAR function to (re)initialize rendering after first use
		// or after OpenGL ES context was lost (e.g. after onPause/onResume):
		QCAR.onSurfaceCreated();
	}

	/** Called when the surface changed size. */
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		DebugLog.LOGD("GLRenderer::onSurfaceChanged");

		// Call native function to update rendering when render surface
		// parameters have changed:
		updateRendering(width, height);
		// Call QCAR function to handle render surface size changes:
		QCAR.onSurfaceChanged(width, height);
	}

	/**  render function */
	public native void renderFrame();

	/** draws the current frame */
	public void onDrawFrame(GL10 gl) {
		if (!mIsActive)
			return;
		
		renderFrame();
	}
}
