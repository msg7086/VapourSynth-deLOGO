/*
VS_DELOGO Copyright(C) 2003 MakKi, 2014 msg7086

This program is free software; you can redistribute it and / or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.
*/

#include "delogo_interface.h"

void VS_CC
logoInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi)
{
	delogo *d = *(delogo**)instanceData;
	vsapi->setVideoInfo(d->vi, 1, node);
}

void VS_CC
logoFree(void *instanceData, VSCore *core, const VSAPI *vsapi)
{
	delogo *d = (delogo*)instanceData;
	vsapi->freeNode(d->node);
	delete d;
}

const VSFrameRef *VS_CC
logoGetFrame(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi)
{
	delogo *d = *(delogo**)instanceData;
	IScriptEnvironment env(frameCtx, core, vsapi, d->node);
	if (activationReason == arInitial) {
		d->GetFramePre(&env, n);
		return NULL;
	}
	if (activationReason != arAllFramesReady) {
		return NULL;
	}

	return d->GetFrame(&env, n);
}

static delogo* VS_CC
logoCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi, int mode) {
	int err;
	char msg_buff[256] = "DELOGO(" PLUGIN_VERSION "): ";
	char *msg = msg_buff + strlen(msg_buff);

	delogo*d = NULL;
	VSNodeRef * node = vsapi->propGetNode(in, "clip", 0, 0);
	VSVideoInfo * vi = new VSVideoInfo;
	*vi = *vsapi->getVideoInfo(node);

	FAIL_IF_ERROR(!vi->format || vi->width == 0 || vi->height == 0,
		"clip must be constant format");

	FAIL_IF_ERROR(vi->format->sampleType != stInteger ||
		vi->format->bitsPerSample != 8 ||
		vi->format->colorFamily != cmYUV,
		"only YUV420P8 input supported. You can you up.");
	{
		PARAM_INT(pos_x, 0);
		PARAM_INT(pos_y, 0);
		PARAM_INT(depth, LOGO_DEFAULT_DEPTH);
		PARAM_INT(yc_y, 0);
		PARAM_INT(yc_u, 0);
		PARAM_INT(yc_v, 0);
		PARAM_INT(start, 0);
		PARAM_INT(end, -1);
		PARAM_INT(fadein, 0);
		PARAM_INT(fadeout, 0);
		PARAM_INT(cutoff, 0);
		PARAM_STR(logofile, NULL);
		PARAM_STR(logoname, NULL);

		try {
			d = new delogo(vsapi, vi, node, logofile, logoname, pos_x, pos_y, depth, yc_y, yc_u, yc_v, start, end, fadein, fadeout, cutoff, mode);
		}
		catch (const char *err) {
			snprintf(msg_buff, 200, err);
			goto fail;
		}
	}
	return d;

fail:
	vsapi->freeNode(node);
	vsapi->setError(out, msg_buff);
	return NULL;
}
static void VS_CC
eraselogoCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	delogo *d = logoCreate(in, out, userData, core, vsapi, -1);
	if (d != NULL)
		vsapi->createFilter(in, out, "eraselogo", logoInit, logoGetFrame, logoFree, fmParallel, 0, d, core);
}

static void VS_CC
addlogoCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
	delogo *d = logoCreate(in, out, userData, core, vsapi, 1);
	if (d != NULL)
		vsapi->createFilter(in, out, "addlogo", logoInit, logoGetFrame, logoFree, fmParallel, 0, d, core);
}

VS_EXTERNAL_API(void)
VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin)
{
	configFunc("in.7086.delogo", "delogo",
		"VapourSynth Delogo Filter v" PLUGIN_VERSION,
		VAPOURSYNTH_API_VERSION, 1, plugin);
	const char * options = "clip:clip;logofile:data;logoname:data:opt;pos_x:int:opt;pos_y:int:opt;depth:int:opt;yc_y:int:opt;yc_u:int:opt;yc_v:int:opt;start:int:opt;end:int:opt;fadein:int:opt;fadeout:int:opt;cutoff:int:opt;";
	registerFunc("eraselogo", options, eraselogoCreate, 0, plugin);
	registerFunc("addlogo", options, addlogoCreate, 0, plugin);
}
