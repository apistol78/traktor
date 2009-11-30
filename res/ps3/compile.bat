@echo off

%SCE_PS3_ROOT%\host-win32\Cg\bin\sce-cgc -profile sce_vp_rsx -o ClearFp_vertex.vpo ClearFp_vertex.cg
%SCE_PS3_ROOT%\host-win32\Cg\bin\sce-cgc -profile sce_fp_rsx -o ClearFp_fragment.fpo ClearFp_fragment.cg
