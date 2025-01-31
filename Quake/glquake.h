/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
Copyright (C) 2010-2014 QuakeSpasm developers
Copyright (C) 2016 Axel Gneiting

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef GLQUAKE_H
#define GLQUAKE_H

void GL_WaitForDeviceIdle(void);
qboolean GL_BeginRendering(int* x, int* y, int* width, int* height);
qboolean GL_AcquireNextSwapChainImage(void);
void GL_EndRendering(qboolean swapchain_acquired);
qboolean GL_Set2D(void);

extern	int glx, gly, glwidth, glheight;

// r_local.h -- private refresh defs

#define ALIAS_BASE_SIZE_RATIO		(1.0 / 11.0)
					// normalizing factor so player model works out to about
					//  1 pixel per triangle
#define	MAX_LBM_HEIGHT		480

#define TILE_SIZE		128		// size of textures generated by R_GenTiledSurf

#define SKYSHIFT		7
#define	SKYSIZE			(1 << SKYSHIFT)
#define SKYMASK			(SKYSIZE - 1)

#define BACKFACE_EPSILON	0.01

#define	MAX_GLTEXTURES	4096

#define NUM_COLOR_BUFFERS 2
#define INITIAL_STAGING_BUFFER_SIZE_KB	16384

#define FAN_INDEX_BUFFER_SIZE 126

#define FRAMES_IN_FLIGHT 2

void R_TimeRefresh_f(void);
void R_ReadPointFile_f(void);
texture_t* R_TextureAnimation(texture_t* base, int frame);

typedef struct surfcache_s
{
	struct surfcache_s* next;
	struct surfcache_s** owner;		// NULL is an empty chunk of memory
	int			lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int			dlight;
	int			size;		// including header
	unsigned		width;
	unsigned		height;		// DEBUG only needed for debug
	float			mipscale;
	struct texture_s* texture;	// checked for animating textures
	byte			data[4];	// width*height elements
} surfcache_t;


typedef struct
{
	pixel_t* surfdat;	// destination for generated surface
	int		rowbytes;	// destination logical width in bytes
	msurface_t* surf;		// description for surface to generate
	fixed8_t	lightadj[MAXLIGHTMAPS];
	// adjust for lightmap levels for dynamic lighting
	texture_t* texture;	// corrected for animating textures
	int		surfmip;	// mipmapped ratio of surface texels / world pixels
	int		surfwidth;	// in mipmapped texels
	int		surfheight;	// in mipmapped texels
} drawsurf_t;


typedef enum {
	pt_static, pt_grav, pt_slowgrav, pt_fire, pt_explode, pt_explode2, pt_blob, pt_blob2
} ptype_t;

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
typedef struct particle_s
{
	// driver-usable fields
	vec3_t		org;
	float		color;
	// drivers never touch the following fields
	struct particle_s* next;
	vec3_t		vel;
	float		ramp;
	float		die;
	ptype_t		type;
} particle_t;

#define P_INVALID -1
#ifdef PSET_SCRIPT
void PScript_InitParticles(void);
void PScript_Shutdown(void);
void PScript_DrawParticles(void);
void PScript_DrawParticles_ShowTris(void);
struct trailstate_s;
int PScript_ParticleTrail(vec3_t startpos, vec3_t end, int type, float timeinterval, int dlkey, vec3_t axis[3], struct trailstate_s** tsk);
int PScript_RunParticleEffectState(vec3_t org, vec3_t dir, float count, int typenum, struct trailstate_s** tsk);
void PScript_RunParticleWeather(vec3_t minb, vec3_t maxb, vec3_t dir, float count, int colour, const char* efname);
void PScript_EmitSkyEffectTris(qmodel_t* mod, msurface_t* fa, int ptype);
int PScript_FindParticleType(const char* fullname);
int PScript_RunParticleEffectTypeString(vec3_t org, vec3_t dir, float count, const char* name);
int PScript_EntParticleTrail(vec3_t oldorg, entity_t* ent, const char* name);
int PScript_RunParticleEffect(vec3_t org, vec3_t dir, int color, int count);
void PScript_DelinkTrailstate(struct trailstate_s** tsk);
void PScript_ClearParticles(void);
void PScript_UpdateModelEffects(qmodel_t* mod);
void PScript_ClearSurfaceParticles(qmodel_t* mod);	//model is being unloaded.

extern int r_trace_line_cache_counter;
#define InvalidateTraceLineCache() do { ++r_trace_line_cache_counter; } while(0);
#else
#define PScript_RunParticleEffectState(o,d,c,t,s) true
#define PScript_RunParticleEffectTypeString(o,d,c,n) true	//just unconditionally returns an error
#define PScript_EntParticleTrail(o,e,n) true
#define PScript_ParticleTrail(o,e,t,d,a,s) true
#define PScript_EntParticleTrail(o,e,n) true
#define PScript_RunParticleEffect(o,d,p,c) true
#define PScript_RunParticleWeather(min,max,d,c,p,n)
#define PScript_ClearSurfaceParticles(m)
#define PScript_DelinkTrailstate(tsp)
#define InvalidateTraceLineCache()
#endif

// Ray generation shader structs

typedef struct raygen_uniform_data_s {
	uint32_t maxDepth;
	uint32_t maxSamples;
	uint32_t frame;
} raygen_uniform_data_t;

typedef struct raygen_push_constants_s {
	float view_inverse[16];
	float proj_inverse[16];
} raygen_push_constants_t;

typedef struct BufferResource_s {
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDeviceAddress address;
	size_t size;
	int is_mapped;
} BufferResource_t;

typedef struct accel_match_info_s {
	int fast_build;
	uint32_t vertex_count;
	uint32_t index_count;
	uint32_t aabb_count;
	uint32_t instance_count;
} accel_match_info_t;

typedef struct accel_struct_s {
	VkAccelerationStructureKHR accel;
	accel_match_info_t match;
	BufferResource_t mem;
	qboolean present;
} accel_struct_t;

typedef struct blas_instances_s {
	accel_struct_t static_blas;
	accel_struct_t dynamic_blas;
} blas_instances_t;

typedef struct raygen_uniform_second_s {
	int* texture_index;
}raygen_uniform_second_t;

typedef struct model_material_s {
	VkImageView tx_imageview;
	VkImageView fb_imageview;
} model_material_t;

typedef struct rt_vertex_s {
	float vertex_pos[3];
	float vertex_tx_coords[2];
	float vertex_fb_coords[2];
	int	tx_index;
	int fb_index;
	int material_index;
} rt_vertex_t;

typedef struct rt_data_s {
	/*size_t* blas_instances_count;
	size_t* blas_instances_size;
	VkAccelerationStructureInstanceKHR** blas_instances;*/

	size_t* vertex_data_count;
	size_t* vertex_data_size;
	rt_vertex_t** vertex_data;

	size_t* index_data_count;
	size_t* index_data_size;
	uint32_t** index_data;

	size_t* geometry_count;
	size_t* geometry_index_data_size;
	uint32_t** geometry_index_offsets_data;

	//uint32_t** texture_index_data;

	size_t* texture_data_count;
	size_t* texture_data_size;
	model_material_t** texture_data;
} rt_data_t;

typedef struct rt_blas_data_t {
	int vertex_buffer_offset;
	int vertex_count;
	int index_buffer_offset;
	int index_count;
	int model_count;
	int model_info_buffer_offset;
	VkBuffer transform_data_buffer;
} rt_blas_data_t;

typedef struct rt_blas_shader_data_s {
	int vertex_buffer_offset;
	int index_buffer_offset;
} rt_blas_shader_data_t;

typedef struct rt_model_shader_data_s {
	int texture_buffer_offset_index;
	int texture_buffer_fullbright_offset_index;
} rt_model_shader_data_t;

typedef struct rt_light_entity_s {
	vec4_t origin_radius;
	vec4_t light_color;
	vec3_t absmin;
	vec3_t absmax;
	vec4_t light_clamp;

	int leafnums[16];
	int lightStyle;

	int num_leafs;
	float distance;

	int index;

	//qboolean isAreaLight;
} rt_light_entity_t;

typedef struct rt_light_entity_shader_s {
	vec4_t origin_radius;
	vec4_t light_color;
	vec4_t light_clamp;
} rt_light_entity_shader_t;

typedef struct vulkan_pipeline_layout_s {
	VkPipelineLayout		handle;
	VkPushConstantRange		push_constant_range;
} vulkan_pipeline_layout_t;

typedef struct vulkan_pipeline_s {
	VkPipeline					handle;
	vulkan_pipeline_layout_t	layout;
} vulkan_pipeline_t;

typedef struct vulkan_desc_set_layout_s {
	VkDescriptorSetLayout		handle;
	int							num_combined_image_samplers;
	int							num_ubos_dynamic;
	int							num_input_attachments;
	int							num_storage_vertex;
	int							num_tlas;
	int							num_storage_images;
} vulkan_desc_set_layout_t;

#define WORLD_PIPELINE_COUNT 8
#define FTE_PARTICLE_PIPELINE_COUNT 16

typedef struct
{
	VkDevice							device;
	qboolean							device_idle;
	qboolean							validation;
	qboolean							debug_utils;
	VkQueue								queue;
	VkCommandBuffer						command_buffer;
	int									current_command_buffer;
	vulkan_pipeline_t					current_pipeline;
	VkClearValue						color_clear_value;
	VkFormat							swap_chain_format;
	qboolean							want_full_screen_exclusive;
	qboolean							swap_chain_full_screen_exclusive;
	qboolean							swap_chain_full_screen_acquired;
	VkPhysicalDeviceProperties			device_properties;
	VkPhysicalDeviceMemoryProperties	memory_properties;
	uint32_t							gfx_queue_family_index;
	VkFormat							color_format;
	VkFormat							depth_format;
	VkSampleCountFlagBits				sample_count;
	qboolean							supersampling;
	qboolean							non_solid_fill;
	qboolean							screen_effects_sops;

	blas_instances_t					blas_instances[FRAMES_IN_FLIGHT];

	// TLAS
	accel_struct_t						tlas_instances[FRAMES_IN_FLIGHT];

	// Scratch buffer
	int									scratch_buffer_pointer;
	BufferResource_t					acceleration_structure_scratch_buffer;
	
	VkImageView							output_image_view[FRAMES_IN_FLIGHT];
	//VkImageView							output_image_view;

	// RT Buffers
	int									as_instances_pointer;
	BufferResource_t					as_instances[FRAMES_IN_FLIGHT];

	// TODO: Replace most buffers with the dynamic buffers made in rtquake
	BufferResource_t					rt_static_vertex_buffer_resource;
	int									rt_static_vertex_count;

	VkBuffer							rt_dynamic_vertex_buffer;

	VkDeviceMemory						rt_static_index_memory;
	VkBuffer							rt_static_index_buffer;
	int									rt_static_index_count;

	VkBuffer							rt_dynamic_index_buffer;

	BufferResource_t					rt_uniform_buffer;

	int									rt_current_blas_index;
	rt_blas_data_t*						rt_blas_data_pointer;

	int									rt_light_entities_count;
	rt_light_entity_t*					rt_light_entities;

	BufferResource_t					rt_light_entities_buffer;
	BufferResource_t					rt_light_entities_list_buffer;

	VkDescriptorImageInfo*				texture_list;
	int									texture_list_count;

	// Instance extensions
	qboolean							get_surface_capabilities_2;
	qboolean							get_physical_device_properties_2;
	qboolean							vulkan_1_2_available;

	// Device extensions
	qboolean							dedicated_allocation;
	qboolean							full_screen_exclusive;
	qboolean							ray_pipeline;
	qboolean							ray_query;

	// Properties
	VkPhysicalDeviceAccelerationStructurePropertiesKHR acceleration_structure_properties;
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR	raytracing_pipeline_properties;

	// Buffers
	VkImage								color_buffers[NUM_COLOR_BUFFERS];
	VkImage								ray_image_buffers;

	// Index buffers
	VkBuffer							fan_index_buffer;

	// Staging buffers
	int									staging_buffer_size;

	// Device procedures
	PFN_vkCreateRayTracingPipelinesKHR				fpCreateRayTracingPipelinesKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR		fpGetRayTracingShaderGroupHandlesKHR;
	PFN_vkCmdTraceRaysKHR							fpCmdTraceRaysKHR;

	PFN_vkGetAccelerationStructureBuildSizesKHR		fpGetAccelerationStructureBuildSizesKHR;
	PFN_vkCreateAccelerationStructureKHR			fpCreateAccelerationStructureKHR;
	PFN_vkDestroyAccelerationStructureKHR			fpDestroyAccelerationStructureKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR			fpCmdBuildAccelerationStructuresKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR	fpGetAccelerationStructureDeviceAddressKHR;

	// Render passes
	VkRenderPass						main_render_pass;
	VkClearValue						main_clear_values[4];
	VkRenderPassBeginInfo				main_render_pass_begin_infos[2];
	VkRenderPass						raygen_render_pass;
	VkClearValue						raygen_clear_values;
	VkRenderPassBeginInfo				raygen_render_pass_begin_infos[2];
	VkRenderPass						ui_render_pass;
	VkRenderPassBeginInfo				ui_render_pass_begin_info;
	VkRenderPass						warp_render_pass;

	// Pipelines
	vulkan_pipeline_t					basic_alphatest_pipeline[2];
	vulkan_pipeline_t					basic_blend_pipeline[2];
	vulkan_pipeline_t					basic_notex_blend_pipeline[2];
	vulkan_pipeline_t					basic_poly_blend_pipeline;
	vulkan_pipeline_layout_t			basic_pipeline_layout;
	vulkan_pipeline_t					world_pipelines[WORLD_PIPELINE_COUNT];
	vulkan_pipeline_layout_t			world_pipeline_layout;
	vulkan_pipeline_t					water_pipeline;
	vulkan_pipeline_t					water_blend_pipeline;
	vulkan_pipeline_t					raster_tex_warp_pipeline;
	vulkan_pipeline_t					particle_pipeline;
	vulkan_pipeline_t					sprite_pipeline;
	vulkan_pipeline_t					sky_stencil_pipeline;
	vulkan_pipeline_t					sky_color_pipeline;
	vulkan_pipeline_t					sky_box_pipeline;
	vulkan_pipeline_t					sky_layer_pipeline;
	vulkan_pipeline_t					alias_pipeline;
	vulkan_pipeline_t					alias_blend_pipeline;
	vulkan_pipeline_t					alias_alphatest_pipeline;
	vulkan_pipeline_t					alias_alphatest_blend_pipeline;
	vulkan_pipeline_t					postprocess_pipeline;
	vulkan_pipeline_t					screen_effects_pipeline;
	vulkan_pipeline_t					screen_effects_scale_pipeline;
	vulkan_pipeline_t					screen_effects_scale_sops_pipeline;
	vulkan_pipeline_t					cs_tex_warp_pipeline;
	vulkan_pipeline_t					showtris_pipeline;
	vulkan_pipeline_t					showtris_depth_test_pipeline;
	vulkan_pipeline_t					showbboxes_pipeline;
	vulkan_pipeline_t					alias_showtris_pipeline;
	vulkan_pipeline_t					alias_showtris_depth_test_pipeline;
	vulkan_pipeline_t					raygen_pipeline;
#ifdef PSET_SCRIPT
	vulkan_pipeline_t					fte_particle_pipelines[FTE_PARTICLE_PIPELINE_COUNT];
#endif

	// Descriptors
	VkDescriptorPool					descriptor_pool;
	vulkan_desc_set_layout_t			ubo_set_layout;
	vulkan_desc_set_layout_t			single_texture_set_layout;
	vulkan_desc_set_layout_t			input_attachment_set_layout;
	VkDescriptorSet						screen_warp_desc_set;
	vulkan_desc_set_layout_t			screen_warp_set_layout;
	vulkan_desc_set_layout_t			single_texture_cs_write_set_layout;
	vulkan_desc_set_layout_t			model_vertex_set_layout;
	VkDescriptorSet						raygen_desc_set[FRAMES_IN_FLIGHT];
	vulkan_desc_set_layout_t			raygen_set_layout;

	// Ray generation shader regions
	VkStridedDeviceAddressRegionKHR		rt_gen_region;
	VkStridedDeviceAddressRegionKHR		rt_miss_region;
	VkStridedDeviceAddressRegionKHR		rt_hit_region;
	VkStridedDeviceAddressRegionKHR		rt_call_region;

	// Samplers
	VkSampler							point_sampler;
	VkSampler							linear_sampler;
	VkSampler							point_aniso_sampler;
	VkSampler							linear_aniso_sampler;
	VkSampler							point_sampler_lod_bias;
	VkSampler							linear_sampler_lod_bias;
	VkSampler							point_aniso_sampler_lod_bias;
	VkSampler							linear_aniso_sampler_lod_bias;

	// Matrices
	float								projection_matrix[16];
	float								view_matrix[16];
	float								view_projection_matrix[16];

	//Dispatch table
	PFN_vkCmdBindPipeline				vk_cmd_bind_pipeline;
	PFN_vkCmdPushConstants				vk_cmd_push_constants;
	PFN_vkCmdBindDescriptorSets			vk_cmd_bind_descriptor_sets;
	PFN_vkCmdBindIndexBuffer			vk_cmd_bind_index_buffer;
	PFN_vkCmdBindVertexBuffers			vk_cmd_bind_vertex_buffers;
	PFN_vkCmdDraw						vk_cmd_draw;
	PFN_vkCmdDrawIndexed				vk_cmd_draw_indexed;
	PFN_vkCmdPipelineBarrier			vk_cmd_pipeline_barrier;
	PFN_vkCmdCopyBufferToImage			vk_cmd_copy_buffer_to_image;

#ifdef _DEBUG
	PFN_vkCmdBeginDebugUtilsLabelEXT	vk_cmd_begin_debug_utils_label;
	PFN_vkCmdEndDebugUtilsLabelEXT		vk_cmd_end_debug_utils_label;
#endif
} vulkanglobals_t;

extern vulkanglobals_t vulkan_globals;

//====================================================

extern	qboolean	r_cache_thrash;		// compatability
extern	vec3_t		modelorg, r_entorigin;
extern	entity_t* currententity;
extern	int		r_visframecount;	// ??? what difs?
extern	int		r_framecount;
extern	mplane_t	frustum[4];
extern	int render_pass_index;
extern	qboolean render_warp;
extern	qboolean in_update_screen;
extern	qboolean use_simd;
extern int render_scale;

//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_refdef;
extern	mleaf_t* r_viewleaf, * r_oldviewleaf;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	cvar_t	r_drawentities;
extern	cvar_t	r_drawworld;
extern	cvar_t	r_drawviewmodel;
extern	cvar_t	r_speeds;
extern	cvar_t	r_pos;
extern	cvar_t	r_waterwarp;
extern	cvar_t	r_fullbright;
extern	cvar_t	r_lightmap;
extern	cvar_t	r_wateralpha;
extern	cvar_t	r_lavaalpha;
extern	cvar_t	r_telealpha;
extern	cvar_t	r_slimealpha;
extern	cvar_t	r_dynamic;
extern	cvar_t	r_novis;
extern	cvar_t	r_scale;

extern	cvar_t	gl_polyblend;
extern	cvar_t	gl_nocolors;

extern	cvar_t	gl_subdivide_size;

//johnfitz -- polygon offset
#define OFFSET_NONE 0
#define OFFSET_DECAL 1

//johnfitz -- rendering statistics
extern unsigned int rs_brushpolys, rs_aliaspolys, rs_skypolys, rs_particles, rs_fogpolys;
extern unsigned int rs_dynamiclightmaps, rs_brushpasses, rs_aliaspasses, rs_skypasses;
extern float rs_megatexels;

//johnfitz -- track developer statistics that vary every frame
extern cvar_t devstats;
typedef struct {
	int		packetsize;
	int		edicts;
	int		visedicts;
	int		efrags;
	int		tempents;
	int		beams;
	int		dlights;
} devstats_t;
extern devstats_t dev_stats, dev_peakstats;

//ohnfitz -- reduce overflow warning spam
typedef struct {
	double	packetsize;
	double	efrags;
	double	beams;
	double	varstring;
} overflowtimes_t;
extern overflowtimes_t dev_overflows; //this stores the last time overflow messages were displayed, not the last time overflows occured
#define CONSOLE_RESPAM_TIME 3 // seconds between repeated warning messages

typedef struct
{
	float	position[3];
	float	texcoord[2];
	byte	color[4];
} basicvertex_t;

//johnfitz -- moved here from r_brush.c
extern int gl_lightmap_format, lightmap_bytes;

#define LMBLOCK_WIDTH	256	//FIXME: make dynamic. if we have a decent card there's no real reason not to use 4k or 16k (assuming there's no lightstyles/dynamics that need uploading...)
#define LMBLOCK_HEIGHT	256 //Alternatively, use texture arrays, which would avoid the need to switch textures as often.

typedef struct glRect_s {
	unsigned short l, t, w, h;
} glRect_t;
struct lightmap_s
{
	gltexture_t* texture;
	qboolean	modified;
	glRect_t	rectchange;

	// the lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly
	byte* data;//[4*LMBLOCK_WIDTH*LMBLOCK_HEIGHT];
};
extern struct lightmap_s* lightmaps;
extern int lightmap_count;	//allocated lightmaps

extern qboolean r_fullbright_cheatsafe, r_lightmap_cheatsafe, r_drawworld_cheatsafe; //johnfitz

extern float	map_wateralpha, map_lavaalpha, map_telealpha, map_slimealpha; //ericw
extern float	map_fallbackalpha; //spike -- because we might want r_wateralpha to apply to teleporters while water itself wasn't watervised

//johnfitz -- fog functions called from outside gl_fog.c
void Fog_ParseServerMessage(void);
float* Fog_GetColor(void);
float Fog_GetDensity(void);
void Fog_EnableGFog(void);
void Fog_DisableGFog(void);
void Fog_SetupFrame(void);
void Fog_NewMap(void);
void Fog_Init(void);

void R_NewGame(void);

void R_AnimateLight(void);
void R_MarkSurfaces(void);
qboolean R_CullBox(vec3_t emins, vec3_t emaxs);
void R_StoreEfrags(efrag_t** ppefrag);
qboolean R_CullModelForEntity(entity_t* e);
void R_RotateForEntity(float matrix[16], vec3_t origin, vec3_t angles);
void R_MarkLights(dlight_t* light, int num, mnode_t* node);

void R_InitParticles(void);
void R_DrawParticles(void);
void CL_RunParticles(void);
void R_ClearParticles(void);

void R_TranslatePlayerSkin(int playernum);
void R_TranslateNewPlayerSkin(int playernum); //johnfitz -- this handles cases when the actual texture changes
void R_UpdateWarpTextures(void);

void R_DrawWorld(void);
void RT_LoadStaticWorldGeometry();
void RT_LoadDynamicWorldIndices();
void R_DrawAliasModel(entity_t* e);
void R_DrawBrushModel(entity_t* e);
void R_DrawSpriteModel(entity_t* e);

void R_DrawTextureChains_Water(qmodel_t* model, entity_t* ent, texchain_t chain);

void GL_BuildLightmaps(void);
void GL_DeleteBModelVertexBuffer(void);
void GL_BuildBModelVertexBuffer(void);
void GL_BuildBModelRTVertexAndIndexBuffer(void);
void GLMesh_LoadVertexBuffers(void);
void GLMesh_DeleteVertexBuffers(void);

int R_LightPoint(vec3_t p);
void R_InitWorldLightEntities(void);
void R_AddWorldLightEntity(float x, float y, float z, float radius, int lightStyle, float r, float g, float b);
void R_CopyLightEntitiesToBuffer(void);
int lightSort(const void* a, const void* b);
void R_CreateLightEntitiesList(vec3_t viewpos);

void GL_SubdivideSurface(msurface_t* fa);
void R_BuildLightMap(msurface_t* surf, byte* dest, int stride);
void R_RenderDynamicLightmaps(msurface_t* fa);
void R_UploadLightmaps(void);

void R_DrawWorld_ShowTris(void);
void R_DrawBrushModel_ShowTris(entity_t* e);
void R_DrawAliasModel_ShowTris(entity_t* e);
void R_DrawParticles_ShowTris(void);
void R_DrawSpriteModel_ShowTris(entity_t* e);

void DrawGLPoly(glpoly_t* p, float color[3], float alpha);
void GL_MakeAliasModelDisplayLists(qmodel_t* m, aliashdr_t* hdr);

void Sky_Init(void);
void Sky_ClearAll(void);
void Sky_DrawSky(void);
void Sky_NewMap(void);
void Sky_LoadTexture(texture_t* mt);
void Sky_LoadTextureQ64(texture_t* mt);
void Sky_LoadSkyBox(const char* name);

void R_ClearTextureChains(qmodel_t* mod, texchain_t chain);
void R_ChainSurface(msurface_t* surf, texchain_t chain);
void R_DrawTextureChains_RTX(qmodel_t* model, entity_t* ent, texchain_t chain);
void R_DrawTextureChains(qmodel_t* model, entity_t* ent, texchain_t chain);
void R_DrawWorld_Water(void);

void RT_LoadBrushModelIndices(qmodel_t* model, entity_t* ent, texchain_t chain, float mvp[16]);

float GL_WaterAlphaForSurface(msurface_t* fa);

int GL_MemoryTypeFromProperties(uint32_t type_bits, VkFlags requirements_mask, VkFlags preferred_mask);

void R_CreateDescriptorPool();
void R_CreateDescriptorSetLayouts();
void R_InitSamplers();
void R_CreatePipelineLayouts();
void R_CreatePipelines();
void R_DestroyPipelines();

// Utils
VkResult buffer_create(BufferResource_t* buf, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties);
uint32_t get_memory_type(uint32_t mem_req_type_bits, VkMemoryPropertyFlags mem_prop);
VkDeviceAddress get_buffer_device_address(VkBuffer buffer);
VkResult buffer_destroy(BufferResource_t* buf);
void* buffer_map(BufferResource_t* buf);
void buffer_unmap(BufferResource_t* buf);

// path tracing
void R_InitializeRaygenDescriptorSets();
VkResult R_UpdateRaygenDescriptorSets();

// Acceleration structures
// Creates bottom level acceleration strucuture (BLAS)
void RT_Create_BLAS_Instance(accel_struct_t* accel_struct, VkBuffer vertex_buffer,
	uint32_t vertex_offset, uint32_t num_vertices, uint32_t num_triangles, uint32_t stride,
	VkBuffer index_buffer, uint32_t num_indices, uint32_t index_offset, VkFormat format, VkIndexType index_type, VkBuffer transform_data);
void R_Create_TLAS(int num_instances);
int accel_matches(accel_match_info_t* match, int fast_build,uint32_t vertex_count,uint32_t index_count);
int accel_matches_top_level(accel_match_info_t* match, int fast_build, uint32_t instance_count);
void destroy_accel_struct(accel_struct_t* blas);


#define MAX_PUSH_CONSTANT_SIZE 128 // Vulkan guaranteed minimum maxPushConstantsSize

static inline void R_BindPipeline(VkPipelineBindPoint bind_point, vulkan_pipeline_t pipeline)
{
	static byte zeroes[MAX_PUSH_CONSTANT_SIZE];
	assert(pipeline.handle != VK_NULL_HANDLE);
	assert(pipeline.layout.handle != VK_NULL_HANDLE);
	assert(vulkan_globals.current_pipeline.layout.push_constant_range.size <= MAX_PUSH_CONSTANT_SIZE);
	if (vulkan_globals.current_pipeline.handle != pipeline.handle) {
		vulkan_globals.vk_cmd_bind_pipeline(vulkan_globals.command_buffer, bind_point, pipeline.handle);
		if ((vulkan_globals.current_pipeline.layout.push_constant_range.stageFlags != pipeline.layout.push_constant_range.stageFlags)
			|| (vulkan_globals.current_pipeline.layout.push_constant_range.size != pipeline.layout.push_constant_range.size))
			vulkan_globals.vk_cmd_push_constants(vulkan_globals.command_buffer, pipeline.layout.handle, pipeline.layout.push_constant_range.stageFlags, 0, pipeline.layout.push_constant_range.size, zeroes);
		vulkan_globals.current_pipeline = pipeline;
	}
	else {
		vulkan_globals.vk_cmd_bind_pipeline(vulkan_globals.command_buffer, bind_point, pipeline.handle);
	}
}

static inline void R_PushConstants(VkShaderStageFlags stage_flags, int offset, int size, const void* data)
{
	vulkan_globals.vk_cmd_push_constants(vulkan_globals.command_buffer, vulkan_globals.current_pipeline.layout.handle, stage_flags, offset, size, data);
}

static inline void R_BeginDebugUtilsLabel(const char* name)
{
#ifdef _DEBUG
	VkDebugUtilsLabelEXT label;
	memset(&label, 0, sizeof(label));
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	label.pLabelName = name;
	if (vulkan_globals.vk_cmd_begin_debug_utils_label)
		vulkan_globals.vk_cmd_begin_debug_utils_label(vulkan_globals.command_buffer, &label);
#endif
}


static inline void R_EndDebugUtilsLabel()
{
#ifdef _DEBUG
	if (vulkan_globals.vk_cmd_end_debug_utils_label)
		vulkan_globals.vk_cmd_end_debug_utils_label(vulkan_globals.command_buffer);
#endif
}

VkDescriptorSet R_AllocateDescriptorSet(vulkan_desc_set_layout_t* layout);
void R_FreeDescriptorSet(VkDescriptorSet desc_set, vulkan_desc_set_layout_t* layout);

void R_InitStagingBuffers();
void R_SubmitStagingBuffers();
byte* R_StagingAllocate(int size, int alignment, VkCommandBuffer* command_buffer, VkBuffer* buffer, int* buffer_offset);

void R_InitGPUBuffers();
void R_SwapDynamicBuffers();
void R_FlushDynamicBuffers();
void R_CollectDynamicBufferGarbage();
void R_CollectMeshBufferGarbage();
byte* R_VertexAllocate(int size, VkBuffer* buffer, VkDeviceSize* buffer_offset);
byte* R_IndexAllocate(int size, VkBuffer* buffer, VkDeviceSize* buffer_offset);
byte* R_UniformAllocate(int size, VkBuffer* buffer, uint32_t* buffer_offset, VkDescriptorSet* descriptor_set);

void GL_SetObjectName(uint64_t object, VkObjectType object_type, const char* name);

#endif	/* GLQUAKE_H */
