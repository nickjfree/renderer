#ifndef __OPCODE__
#define __OPCODE__


/*
	render target
*/
#define OP_RENDER_TARGET 0x01
#define OP_DEPTH_STENCIL 0x02


/*
	shaders
*/
#define OP_SHADER_VS  0x03
#define OP_SHADER_GS  0x04
#define OP_SHADER_HS  0x05
#define OP_SHADER_DS  0x06
#define OP_SHADER_NOP 0x07
#define OP_SHADER_PS  0x08

/*
	constant
*/
#define OP_SET_CONSTANT 0x09
#define OP_UP_CONSTANT  0x0a
#define OP_UP_ARRAY     0x19

/*
	render stats
*/
#define OP_DEPTH_STAT  0x0b
#define OP_RASTER_STAT 0x0c
#define OP_BLEND_STAT  0x0d


/*
	textures
*/
#define OP_TEXTURE 0x0e

/*
	shader buffer resource
*/

#define OP_BUFFER 0x1a
/*
	shader buffer uav
*/

#define OP_UAVBUF 0x1b
/*
	shader texture uav
*/

#define OP_UAVTEX 0x1c

/*
	geometry
*/
#define OP_RENDER_GEO 0x0f
#define OP_QUAD       0x17
#define OP_INSTANCE   0x18
/*
	frame ops
*/
#define OP_CLEAR_TARGET 0x10
#define OP_END_TARGET   0x11
#define OP_END_EXECUTE  0x12
#define OP_CLEAR_DEPTH   0x15
#define OP_PRESENT 0x16

/*
	layout
*/
#define OP_INPUT_LAYOUT 0x13

/*
	viewport
*/
#define OP_VIEWPORT 0x14

/*
	raytracing
*/
#define OP_BUILD_RTSCENE 0x20
#define OP_TRACERAY      0x21
#define OP_TLAS          0x22

#endif