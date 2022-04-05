// Copyright (c) 2010-2022, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. All Rights reserved. See files
// LICENSE and NOTICE for details. LLNL-CODE-806117.
//
// This file is part of the MFEM library. For more information and source code
// availability visit https://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the BSD-3 license. We welcome feedback and contributions, see file
// CONTRIBUTING.md for details.

#include "qspace.hpp"

namespace mfem
{

QuadratureSpaceBase::QuadratureSpaceBase(Mesh &mesh_, Geometry::Type geom,
                                         const IntegrationRule &ir)
   : mesh(mesh_)
{
   for (int g = 0; g < Geometry::NumGeom; g++)
   {
      int_rule[g] = NULL;
   }
   int_rule[geom] = &ir;
}

void QuadratureSpaceBase::ConstructIntRules(int dim)
{
   Array<Geometry::Type> geoms;
   mesh.GetGeometries(dim, geoms);
   for (Geometry::Type geom : geoms)
   {
      int_rule[geom] = &IntRules.Get(geom, order);
   }
}

void QuadratureSpace::ConstructOffsets()
{
   const int num_elem = mesh.GetNE();
   offsets.SetSize(num_elem + 1);
   int offset = 0;
   for (int i = 0; i < num_elem; i++)
   {
      offsets[i] = offset;
      int geom = mesh.GetElementBaseGeometry(i);
      MFEM_ASSERT(int_rule[geom] != NULL, "Missing integration rule.");
      offset += int_rule[geom]->GetNPoints();
   }
   offsets[num_elem] = size = offset;
}

void QuadratureSpace::Construct()
{
   ConstructIntRules(mesh.Dimension());
   ConstructOffsets();
}

QuadratureSpace::QuadratureSpace(Mesh *mesh_, std::istream &in)
   : QuadratureSpaceBase(*mesh_)
{
   const char *msg = "invalid input stream";
   std::string ident;

   in >> ident; MFEM_VERIFY(ident == "QuadratureSpace", msg);
   in >> ident; MFEM_VERIFY(ident == "Type:", msg);
   in >> ident;
   if (ident == "default_quadrature")
   {
      in >> ident; MFEM_VERIFY(ident == "Order:", msg);
      in >> order;
   }
   else
   {
      MFEM_ABORT("unknown QuadratureSpace type: " << ident);
      return;
   }

   Construct();
}

QuadratureSpace::QuadratureSpace(Mesh &mesh_, const IntegrationRule &ir)
   : QuadratureSpaceBase(mesh_, mesh_.GetElementGeometry(0), ir)
{
   ConstructOffsets();
}

void QuadratureSpace::Save(std::ostream &os) const
{
   os << "QuadratureSpace\n"
      << "Type: default_quadrature\n"
      << "Order: " << order << '\n';
}

FaceQuadratureSpace::FaceQuadratureSpace(Mesh &mesh_, const IntegrationRule &ir)
   : QuadratureSpaceBase(mesh_, mesh_.GetFaceGeometry(0), ir)
{
   ConstructOffsets();
}

void FaceQuadratureSpace::ConstructOffsets()
{
   const int num_faces = mesh.GetNumFaces();
   offsets.SetSize(num_faces + 1);
   int offset = 0;
   for (int i = 0; i < num_faces; i++)
   {
      offsets[i] = offset;
      Geometry::Type geom = mesh.GetFaceGeometry(i);
      MFEM_ASSERT(int_rule[geom] != NULL, "Missing integration rule");
      offset += int_rule[geom]->GetNPoints();
   }
   offsets[num_faces] = size = offset;
}

void FaceQuadratureSpace::Construct()
{
   ConstructIntRules(mesh.Dimension() - 1);
   ConstructOffsets();
}

} // namespace mfem
