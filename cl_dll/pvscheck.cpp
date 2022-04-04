#include "hud.h"
#include "cl_util.h"

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "com_model.h"

#include "glmanager.h"
#include "windows.h"

typedef struct leaflist_s
{
	int		count;
	int		maxcount;
	qboolean		overflowed;
	short		*list;
	vec3_t		mins, maxs;
	int		topnode;		// for overflows where each leaf can't be stored individually
} leaflist_t;

model_t *world;
byte *visdata;
extern vec3_t render_origin;

int BoxOnPlaneSide (vec3_t &emins, vec3_t &emaxs, mplane_t *p);
static void Mod_BoxLeafnums_r( leaflist_t *ll, mnode_t *node )
{
	mplane_t	*plane;
	int	s;

	while( 1 )
	{
		if( node->contents == CONTENTS_SOLID )
			return;

		if( node->contents < 0 )
		{
			mleaf_t	*leaf = (mleaf_t *)node;

			// it's a leaf!
			if( ll->count >= ll->maxcount )
			{
				ll->overflowed = true;
				return;
			}

			ll->list[ll->count++] = leaf - world->leafs - 1;
			return;
		}
	
		plane = node->plane;
		s = BoxOnPlaneSide( ll->mins, ll->maxs, plane );

		if( s == 1 )
		{
			node = node->children[0];
		}
		else if( s == 2 )
		{
			node = node->children[1];
		}
		else
		{
			// go down both
			if( ll->topnode == -1 )
				ll->topnode = node - world->nodes;
			Mod_BoxLeafnums_r( ll, node->children[0] );
			node = node->children[1];
		}
	}
}

#define MAX_BOX_LEAFS		256
int Mod_BoxLeafnums( const vec3_t mins, const vec3_t maxs, short *list, int listsize, int *topnode )
{
	leaflist_t	ll;

	if( !world ) return 0;

	VectorCopy( mins, ll.mins );
	VectorCopy( maxs, ll.maxs );
	ll.count = 0;
	ll.maxcount = listsize;
	ll.list = list;
	ll.topnode = -1;
	ll.overflowed = false;

	Mod_BoxLeafnums_r( &ll, world->nodes );

	if( topnode ) *topnode = ll.topnode;
	return ll.count;
}

/*
===================
Mod_DecompressVis
===================
*/
byte *Mod_DecompressVis( const byte *in )
{
	int	c, row;
	byte	*out;

	if( !world )
	{
		return NULL;
	}

	row = (world->numleafs + 7) >> 3;	
	out = visdata;

	if (!visdata)
		return NULL;

	if( !in )
	{	
		// no vis info, so make all visible
		while( row )
		{
			*out++ = 0xff;
			row--;
		}
		return visdata;
	}

	do
	{
		if( *in )
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;

		while( c )
		{
			*out++ = 0;
			c--;
		}
	} while( out - visdata < row );

	return visdata;
}

byte *Mod_LeafPVS( mleaf_t *leaf )
{
	model_t *model = world;
	if( !model || !leaf || leaf == model->leafs || !model->visdata )
		return Mod_DecompressVis( NULL );
	return Mod_DecompressVis( leaf->compressed_vis );
}

byte *GetCurrentPVS( )
{
	model_t *worldmdl = gEngfuncs.GetEntityByIndex(0)->model;
	if(!worldmdl)
		return NULL;

	mleaf_t *leaf = Mod_PointInLeaf ( render_origin, worldmdl );
	return Mod_LeafPVS(leaf);
}

int GetCurrentVisFrame()
{
	model_t *worldmdl = gEngfuncs.GetEntityByIndex(0)->model;
	if (!worldmdl)
		return NULL;

	mleaf_t *leaf = Mod_PointInLeaf(render_origin, worldmdl);
	if (!leaf) return 0;

	return leaf->visframe;
}

bool BoxInPVS(vec3_t mins,vec3_t maxs)
{
	model_t *worldmdl = gEngfuncs.GetEntityByIndex(0)->model;
	if(!worldmdl)
		return FALSE;

	world = worldmdl;
	visdata = world->visdata;

	short	leafList[MAX_BOX_LEAFS];
	int	i, count;

	byte *visbits = gHUD.currentPVS;
	if( !visbits || !mins || !maxs )
		return TRUE;

	count = Mod_BoxLeafnums( mins, maxs, leafList, MAX_BOX_LEAFS, NULL );

	for( i = 0; i < count; i++ )
	{
		int	leafnum = leafList[i];

		if( leafnum != -1 && visbits[leafnum>>3] & (1<<( leafnum & 7 )))
			return true;
	}
	return false;
}
