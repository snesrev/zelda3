#pragma once
#include "types.h"

uint16 Poly_Divide(uint16 a, uint16 b);
void Poly_RunFrame();
void Polyhedral_SetShapePointer();
void Polyhedral_SetRotationMatrix();
void Polyhedral_OperateRotation();
void Polyhedral_RotatePoint();
void Polyhedral_ProjectPoint();
void Polyhedral_DrawPolyhedron();
void Polyhedral_SetForegroundColor();
int16 Polyhedral_CalculateCrossProduct();
void Polyhedral_SetColorMask(int c);
void Polyhedral_EmptyBitMapBuffer();
void Polyhedral_DrawFace();
void Polyhedral_FillLine();
bool Polyhedral_SetLeft();
bool Polyhedral_SetRight();
