#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Tue Aug 25 07:46:15 2020
#  Last Modified : <200825.0857>
#
#  Description	
#
#  Notes
#
#  History
#	
#*****************************************************************************
#
#    Copyright (C) 2020  Robert Heller D/B/A Deepwoods Software
#			51 Locke Hill Road
#			Wendell, MA 01379-9728
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# 
#
#*****************************************************************************




import Part
from FreeCAD import Base
import FreeCAD as App
import os
import sys
sys.path.append(os.path.dirname(__file__))

from abc import ABCMeta, abstractmethod, abstractproperty

def MM2Mils(MM):
    return (MM/25.4)*1000
    
class LargePiezo(object):
    _outerdiameter = MM2Mils(30)
    _holespacing   = MM2Mils(35)
    @staticmethod
    def _holes(i):
        if i == 0:
            return Base.Vector(0,-(LargePiezo._holespacing/2.0),0)
        elif i == 1:
            return Base.Vector(0,(LargePiezo._holespacing/2.0),0)
        else:
            raise RuntimeError("Hole index out of range!")
    _flangewidth = MM2Mils(5)
    _flangelength = MM2Mils(41)
    @staticmethod
    def _flangeYoffset():
        return (-(LargePiezo._outerdiameter/2.0))-\
                ((LargePiezo._flangelength - LargePiezo._outerdiameter) / 2.0)
    _flangethickness = MM2Mils(3.2)
    _totalthickness  = MM2Mils(5.7)
    _mholeradius     = MM2Mils(1.5) / 2.0
    def MountingHole(self,index,zMount=None,height=None):
        if height == None:
            height = -1 * self._flangethickness
        if zMount == None:
            zMount = self.origin.z
        p = self.origin.add(self._holes(index))
        horig = Base.Vector(p.x,p.y,zMount)
        return Part.Face(Part.Wire(Part.makeCircle(self._mholeradius,horig))\
                        ).extrude(Base.Vector(0,0,height))
    def BodyHole(self,zPanel,height):
        holeorig = Base.Vector(self.origin.x,self.origin.y,zPanel)
        return Part.Face(Part.Wire(Part.makeCircle(self._outerdiameter/2.0,\
                                                   holeorig))\
                        ).extrude(Base.Vector(0,0,height))
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self.body = Part.Face(Part.Wire(\
                        Part.makeCircle(self._outerdiameter/2.0,\
                                        self.origin.add(Base.Vector(0,0,\
                                                    -self._flangethickness))))\
                             ).extrude(Base.Vector(0,0,self._totalthickness))
        self.body = self.body.fuse(\
            Part.makePlane(self._flangewidth,self._flangelength,\
                    self.origin.add(Base.Vector(-(self._flangewidth/2.0),\
                                                self._flangeYoffset(),\
                                                0))\
                          ).extrude(Base.Vector(0,0,-self._flangethickness)))
        self.body = self.body.cut(self.MountingHole(0))
        self.body = self.body.cut(self.MountingHole(1))
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name)
        obj.Shape = self.body
        obj.Label=self.name
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
                                
