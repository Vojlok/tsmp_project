#include "stdafx.h"
#include "build.h"

float ThreadsProgress[17];
bool ThCompl[17];


#include "xrThreadNew.h"
std::mutex mtx;

const	float	aht_max_edge	= c_SS_maxsize/2.5f;	// 2.0f;			// 2 m
//const	float	aht_min_edge	= .2f;					// 20 cm
//const	float	aht_min_err		= 16.f/255.f;			// ~10% error

bool	is_CCW	(int _1, int _2)
{
	if (0==_1 && 1==_2)	return true;
	if (1==_1 && 2==_2) return true;
	if (2==_1 && 0==_2)	return true;
	return	false;
}

// Iterate on edges - select longest
int		callback_edge_longest	(Face* F)
{
	float	max_err				= -1;
	int		max_id				= -1;
	for (u32 e=0; e<3; e++)
	{
		Vertex					*V1,*V2;
		F->EdgeVerts			(e,&V1,&V2);
		float len				= V1->P.distance_to	(V2->P);	// len
		if (len<aht_max_edge)	continue;
		if (len>max_err)
		{
			max_err = len;
			max_id	= e;
		}
	}
	return	max_id;
}

void CBuild::xrPhase_AdaptiveHT	()
{
	CDB::COLLIDER	DB;
	DB.ray_options	(0);

	Status			("Tesselating...");
	if (1)
	{
		for (u32 fit=0; fit<g_faces.size(); fit++)	{		// clear split flag from all faces + calculate normals
			g_faces[fit]->flags.bSplitted		= false;
			g_faces[fit]->flags.bLocked			= true;
			g_faces[fit]->CalcNormal			();
		}
		u_Tesselate		(callback_edge_longest,0,0);		// tesselate
	}

	// Tesselate + calculate
	Status			("Precalculating...");
	{
		mem_Compact					();

		// Build model
		FPU::m64r					();
		BuildRapid					(FALSE);

		// Prepare
		FPU::m64r					();
		Status						("Precalculating : base hemisphere ...");
		mem_Compact					();
		Light_prepare				();

		// calc approximate normals for vertices + base lighting
		int VSize = g_vertices.size();	

		clMsg("iterations count %i", VSize);

		for (int i = 0; i < 17; i++)
		{
			ThreadsProgress[i] = 0.f;
			ThCompl[i] = false;
		}

		auto BaseHemisphereThreadProgress = []()
		{			
			while (true)
			{
				float	fProgress = 0;			
			
				for (int i = 1; i < 17; i++) fProgress += ThreadsProgress[i];
				Progress(fProgress/16);

				if (ThCompl[1] && ThCompl[2] && ThCompl[3] && ThCompl[4] && ThCompl[5] 
					&& ThCompl[6] && ThCompl[7] && ThCompl[8] && ThCompl[9] && ThCompl[10] 
					&& ThCompl[11] && ThCompl[12] && ThCompl[13] && ThCompl[14] 
					&& ThCompl[15] && ThCompl[16])
					break;

				std::this_thread::sleep_for(std::chrono::milliseconds(500));				
			}
		};

		auto BaseHemisphereThread = [](const int ThNumber , const int from, const int to, const CDB::COLLIDER DB1)
		{
			std::unique_lock<std::mutex> cs(mtx, std::defer_lock);
			cs.lock();
			clMsg("* THREAD #%i: Started.", ThNumber);
			cs.unlock();

			int diff =(to - from);
			float fStep=(float)1/(float)(diff);
			ThreadsProgress[ThNumber] = 0;

			for (int it = from; it < to; it++)
			{
				base_color_c		vC;
				CDB::COLLIDER DB = DB1;
				Vertex*		V = g_vertices[it];

				V->normalFromAdj();
				LightPoint(&DB, RCAST_Model, vC, V->P, V->N, pBuild->L_static, LP_dont_rgb + LP_dont_sun, 0);

				vC.mul(0.5f);
				V->C._set(vC);		

				ThreadsProgress[ThNumber] += fStep;
			}
			ThCompl[ThNumber] = true;

			cs.lock();
			clMsg("* THREAD #%i: Task Completed.", ThNumber);
			cs.unlock();
		};

		int Idx[15];
		XrThreadNew TR;
		TR.GenCycleValues(VSize);
		TR.FillValues(Idx);
		std::thread thread_progress(BaseHemisphereThreadProgress);

		std::thread thread1	(BaseHemisphereThread,	1, 0,		Idx[0]	, DB);
		std::thread thread2	(BaseHemisphereThread,	2, Idx[0],	Idx[1]	, DB);
		std::thread thread3	(BaseHemisphereThread,	3, Idx[1],	Idx[2]	, DB);
		std::thread thread4	(BaseHemisphereThread,	4, Idx[2],	Idx[3]	, DB);
		std::thread thread5	(BaseHemisphereThread,	5, Idx[3],	Idx[4]	, DB);
		std::thread thread6	(BaseHemisphereThread,	6, Idx[4],	Idx[5]	, DB);
		std::thread thread7	(BaseHemisphereThread,	7, Idx[5],	Idx[6]	, DB);
		std::thread thread8	(BaseHemisphereThread,	8, Idx[6],	Idx[7]	, DB);
		std::thread thread9	(BaseHemisphereThread,	9, Idx[7],	Idx[8]	, DB);
		std::thread thread10(BaseHemisphereThread, 10, Idx[8],	Idx[9]	, DB);
		std::thread thread11(BaseHemisphereThread, 11, Idx[9],	Idx[10]	, DB);
		std::thread thread12(BaseHemisphereThread, 12, Idx[10],	Idx[11]	, DB);
		std::thread thread13(BaseHemisphereThread, 13, Idx[11],	Idx[12]	, DB);
		std::thread thread14(BaseHemisphereThread, 14, Idx[12],	Idx[13]	, DB);
		std::thread thread15(BaseHemisphereThread, 15, Idx[13],	Idx[14]	, DB);
		std::thread thread16(BaseHemisphereThread, 16, Idx[14],	VSize	, DB);
		thread1.join();
		thread2.join();
		thread3.join();
		thread4.join();
		thread5.join();
		thread6.join();
		thread7.join();
		thread8.join();
		thread9.join();
		thread10.join();
		thread11.join();
		thread12.join();
		thread13.join();
		thread14.join();
		thread15.join();
		thread16.join();

		thread_progress.join();
	}




	//////////////////////////////////////////////////////////////////////////
	/*
	Status				("Adaptive tesselation...");
	{
		for (u32 fit=0; fit<g_faces.size(); fit++)	{					// clear split flag from all faces + calculate normals
			g_faces[fit]->flags.bSplitted	= false;
			g_faces[fit]->flags.bLocked		= true;
		}
		u_Tesselate		(callback_edge_error,0,callback_vertex_hemi);	// tesselate
	}
	*/

	//////////////////////////////////////////////////////////////////////////
	Status				("Gathering lighting information...");
	u_SmoothVertColors	(5);

	//////////////////////////////////////////////////////////////////////////
	/*
	Status				("Exporting to SMF...");
	{
		string_path			fn;
		GSaveAsSMF			(strconcat(fn,pBuild->path,"hemi_source.smf"));
	}
	*/
}

void CBuild::u_Tesselate	(tesscb_estimator* cb_E, tesscb_face* cb_F, tesscb_vertex* cb_V)
{
	// main process
	FPU::m64r					();
	Status						("Tesselating...");
	g_bUnregister				= FALSE;
	xr_vector<Face*> adjacent;	adjacent.reserve(6*2*3);
	u32		counter_create		= 0;
	u32		cnt_verts			= g_vertices.size	();
	u32		cnt_faces			= g_faces.size		();
	for (u32 I=0; I<g_faces.size(); I++)
	{
		Face* F					= g_faces[I];
		if (0==F)				continue;
		if (F->flags.bSplitted)	{
			if (!F->flags.bLocked)	FacePool.destroy	(g_faces[I]);
			continue;
		}
		if (F->CalcArea()<EPS_L)	continue;

		Progress				(float(I)/float(g_faces.size()));
		int		max_id			=	cb_E	(F);
		if		(max_id<0)		continue;	// nothing selected

		// now, we need to tesselate all faces which shares this 'problematic' edge
		// collect all this faces
		Vertex			*V1,*V2;
		F->EdgeVerts	(max_id,&V1,&V2);
		adjacent.clear	();
		for (u32 adj=0; adj<V1->adjacent.size(); adj++)
		{
			Face* A					= V1->adjacent[adj];
			if (A->flags.bSplitted)	continue;
			if (A->VContains(V2))	adjacent.push_back	(A);
		}
		std::sort		(adjacent.begin(),adjacent.end());
		adjacent.erase	(std::unique(adjacent.begin(),adjacent.end()),adjacent.end());

		// create new vertex (lerp)
		counter_create		++;
		if (0==(counter_create%10000))	{
			for (u32 I=0; I<g_vertices.size(); I++)	if (0==g_vertices[I]->adjacent.size())	VertexPool.destroy	(g_vertices[I]);
			Status				("Working: %d verts created, %d(now) / %d(was) ...",counter_create,g_vertices.size(),cnt_verts);
			FlushLog			();
		}

		Vertex*		V		= VertexPool.create();
		V->P.lerp			(V1->P, V2->P, .5f);

		// iterate on faces which share this 'problematic' edge
		for (u32 af_it=0; af_it<adjacent.size(); af_it++)
		{
			Face*	AF			= adjacent[af_it];
			VERIFY				(false==AF->flags.bSplitted);
			AF->flags.bSplitted	= true;
			_TCF&	atc			= AF->tc.front();

			// indices & tc
			int id1				= AF->VIndex(V1);
			int id2				= AF->VIndex(V2);
			int idB				= 3-(id1+id2);
			Fvector2			UV;
			UV.averageA			(atc.uv[id1],atc.uv[id2]);

			// Create F1 & F2
			Face* F1			= FacePool.create();
			F1->flags.bSplitted	= false;
			F1->flags.bLocked	= false;
			F1->dwMaterial		= AF->dwMaterial;
			F1->dwMaterialGame	= AF->dwMaterialGame;
			Face* F2			= FacePool.create();
			F2->flags.bSplitted	= false;
			F2->flags.bLocked	= false;
			F2->dwMaterial		= AF->dwMaterial;
			F2->dwMaterialGame	= AF->dwMaterialGame;

			if (is_CCW(id1,id2))	
			{
				// F1
				F1->SetVertices		(AF->v[idB],	AF->v[id1],		V);
				F1->AddChannel		(atc.uv[idB],	atc.uv[id1],	UV);
				// F2
				F2->SetVertices		(AF->v[idB],	V,				AF->v[id2]);
				F2->AddChannel		(atc.uv[idB],	UV,				atc.uv[id2]);
			} else {
				// F1
				F1->SetVertices		(AF->v[idB],	V,				AF->v[id1]);
				F1->AddChannel		(atc.uv[idB],	UV,				atc.uv[id1]);
				// F2
				F2->SetVertices		(AF->v[idB],	AF->v[id2],		V);
				F2->AddChannel		(atc.uv[idB],	atc.uv[id2],	UV);
			}

			// Normals and checkpoint
			F1->N	= AF->N;		if (cb_F)	cb_F(F1);
			F2->N	= AF->N;		if (cb_F)	cb_F(F2);

			// don't destroy old face	(it can be used as occluder during ray-trace)
			// if (AF->bLocked)	continue;
			// FacePool.destroy	(g_faces[I]);
		}

		// calc vertex attributes
		{
			V->normalFromAdj		();
			if (cb_V)				cb_V	(V);
		}
	}

	// Cleanup
	for (u32 I=0; I<g_faces.size(); I++)	if (0!=g_faces[I] && g_faces[I]->flags.bSplitted)	FacePool.destroy	(g_faces[I]);
	for (u32 I=0; I<g_vertices.size(); I++)	if (0==g_vertices[I]->adjacent.size())				VertexPool.destroy	(g_vertices[I]);
	g_faces.erase		(std::remove(g_faces.begin(),g_faces.end(),(Face*)0),g_faces.end());
	g_vertices.erase	(std::remove(g_vertices.begin(),g_vertices.end(),(Vertex*)0),g_vertices.end());
	g_bUnregister		= true;
}

void CBuild::u_SmoothVertColors(int count)
{
	for (int iteration=0; iteration<count; iteration++)
	{
		// Gather
		xr_vector<base_color>	colors;
		colors.resize			(g_vertices.size());
		for (u32 it=0; it<g_vertices.size(); it++)
		{
			// Circle
			xr_vector<Vertex*>	circle;
			Vertex*		V		= g_vertices[it];
			for (u32 fit=0; fit<V->adjacent.size(); fit++)	{
				Face*	F		= V->adjacent[fit];
				circle.push_back(F->v[0]);
				circle.push_back(F->v[1]);
				circle.push_back(F->v[2]);
			}
			std::sort			(circle.begin(),circle.end());
			circle.erase		(std::unique(circle.begin(),circle.end()),circle.end());

			// Average
			base_color_c		avg,tmp;
			for (u32 cit=0; cit<circle.size(); cit++)
			{
				circle[cit]->C._get	(tmp);
				avg.add				(tmp);
			}
			avg.scale			(circle.size());
			colors[it]._set		(avg);
		}

		// Transfer
		for (u32 it=0; it<g_vertices.size(); it++)
			g_vertices[it]->C	= colors[it];
	}
}
