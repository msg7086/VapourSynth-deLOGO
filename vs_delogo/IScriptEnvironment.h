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

#pragma once
#include "delogo_interface.h"
enum {PLANAR_Y, PLANAR_U, PLANAR_V};

class IScriptEnvironment
{
public:
	VSFrameContext *frameCtx;
	VSCore *core;
	const VSAPI *vsapi;
	VSNodeRef *node;
	const VSVideoInfo *vi;

	IScriptEnvironment(VSFrameContext *_frameCtx, VSCore *_core, const VSAPI *_vsapi, VSNodeRef *_node)
		: frameCtx(_frameCtx), core(_core), vsapi(_vsapi), node(_node) {
		vi = vsapi->getVideoInfo(node);
	}
	~IScriptEnvironment() { }
	VSFrameRef *NewVideoFrame(const VSVideoInfo * vi) {
		return vsapi->newVideoFrame(vi->format, vi->width, vi->height, nullptr, core);
	}
	const VSFrameRef *GetFrame(int n) {
		return vsapi->getFrame(n, node, nullptr, 0);
	}
	void FreeFrame(const VSFrameRef* source) {
		vsapi->freeFrame(source);
	}

	VSFrameRef * MakeWritable(const VSFrameRef* source) {
		return vsapi->copyFrame(source, core);
	}
	void BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height) {
		if ((!height) || (!row_size)) return;

		if (height == 1 || (dst_pitch == src_pitch && src_pitch == row_size)) {
			memcpy(dstp, srcp, row_size*height);
		}
		else {
			for (int y = height; y > 0; --y) {
				memcpy(dstp, srcp, row_size);
				dstp += dst_pitch;
				srcp += src_pitch;
			}
		}
	}
	void PrefetchFrame(int n) {
		vsapi->requestFrameFilter(n, node, frameCtx);
	}
	int GetRowSize(const VSFrameRef* frame) { return vi->width; } // FIXME: row-size = width * sizeof(size_t)
	int GetHeight(const VSFrameRef* frame) { return vi->height; }
	int GetPitch(const VSFrameRef* frame, int plane = PLANAR_Y) { return vsapi->getStride(frame, plane); }
	BYTE* GetWritePtr(VSFrameRef* frame, int plane = PLANAR_Y) { return vsapi->getWritePtr(frame, plane); }
};

