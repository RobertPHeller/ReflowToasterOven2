#!/usr/local/bin/FreeCAD018                                                     
#*****************************************************************************
#
#  System        : 
#  Module        : 
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Robert Heller
#  Created       : Wed Aug 26 10:27:50 2020
#  Last Modified : <200826.1317>
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
    
class HeatSinkMain(object):
    _width = MM2Mils(123.27) # Match ham_1591XXTSBK
    _length = MM2Mils(200.938)
    _height = MM2Mils(41.235)
    _fanextra = MM2Mils(128.206-120)
    _fanholespace = MM2Mils(32)
    _fanholeradius = MM2Mils(2.8/2.0)
    _fanholeZoff = MM2Mils(3.263)
    _fanholeYoff = MM2Mils(3)
    _thickness = 125
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name)
        obj.Shape = self.body
        obj.Label=self.name
        obj.ViewObject.ShapeColor=tuple([.85,.85,.85])
    def cutfrom(self,shape):
        self.body = self.body.cut(shape)
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        zExtrude = Base.Vector(0,0,self._thickness)
        yExtrude = Base.Vector(0,self._thickness,0)
        xExtrude = Base.Vector(self._thickness,0,0)
        yNorm = Base.Vector(0,1,0)
        xNorm = Base.Vector(1,0,0)
        mainpanel = Part.makePlane(self._width,self._length,self.origin\
                                  ).extrude(zExtrude)
        rightOrig = self.origin.add(Base.Vector(0,0,-self._height))
        rightpanel = Part.makePlane(self._height,self._width,rightOrig,yNorm\
                                   ).extrude(yExtrude)
        leftOrig = self.origin.add(Base.Vector(0,self._length-self._thickness,\
                                    -self._height))
        leftpanel = Part.makePlane(self._height,self._width,leftOrig,yNorm\
                                  ).extrude(yExtrude)
        fanmountOrig = leftOrig
        fanmount = Part.makePlane(self._height,self._fanextra,fanmountOrig,\
                                  xNorm).extrude(xExtrude)

        fanhole1Orig = fanmountOrig.add(Base.Vector(0,-self._fanholeYoff,\
                                        self._fanholeZoff))
        fanhole2Orig = fanhole1Orig.add(Base.Vector(0,0,self._fanholespace))
        fanmount = fanmount.cut(Part.Face(\
                    Part.Wire(Part.makeCircle(self._fanholeradius,\
                                              fanhole1Orig,xNorm))\
                                          ).extrude(xExtrude))
        fanmount = fanmount.cut(Part.Face(\
                    Part.Wire(Part.makeCircle(self._fanholeradius,\
                                              fanhole2Orig,xNorm))\
                                          ).extrude(xExtrude))
        self.body = mainpanel.fuse(rightpanel).fuse(leftpanel).fuse(fanmount)
        

class HeatSinkFin(object):
    _lengths = { 1 : MM2Mils(45),
                 2 : MM2Mils(28.527),
                 3 : MM2Mils(12.527) }
    _totallengths = { 1 : MM2Mils(113.938),
                      2 : MM2Mils(98.938),
                      3 : MM2Mils(83.938) }
    _widthA = MM2Mils(90)
    _widthB = MM2Mils(85)
    _thickness = 125
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name)
        obj.Shape = self.body
        obj.Label=self.name
        obj.ViewObject.ShapeColor=tuple([.85,.85,.85])
    def cutfrom(self,shape):
        self.body = self.body.cut(shape)
    def __init__(self,name,origin,finno):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        if finno not in [1,2,3]:
            raise RuntimeError("finno out of range (1,2,3)!")
        self.finno = finno
        zExtrude = Base.Vector(0,0,self._thickness)
        yExtrude = Base.Vector(0,self._thickness,0)
        yNorm = Base.Vector(0,1,0)
        mainpanel = Part.makePlane(self._widthA,self._lengths[finno],\
                                   self.origin).extrude(zExtrude)
        height = (self._totallengths[finno]-self._lengths[finno])/2.0
        rightOrig = self.origin.add(Base.Vector(0,0,-height))
        rightpanel = Part.makePlane(height,self._widthB,rightOrig,yNorm\
                                   ).extrude(yExtrude)
        leftOrig = self.origin.add(Base.Vector(0,\
                                        self._lengths[finno]-self._thickness,\
                                        -height))
        leftpanel = Part.makePlane(height,self._widthB,leftOrig,yNorm\
                                  ).extrude(yExtrude)
        self.body = mainpanel.fuse(rightpanel).fuse(leftpanel)

class HeatSink(object):
    _finXOffset = MM2Mils(12)
    _finCenterYOffset = HeatSinkMain._length-MM2Mils(32.085-2.54)
    _thickness = 125
    def show(self):
        self.mainpanel.show()
        self.fin1.show()
        self.fin2.show()
        self.fin3.show()
    def cutfrom(self,shape):
        self.mainpanel.cutfrom(shape)
        self.fin1.cutfrom(shape)
        self.fin2.cutfrom(shape)
        self.fin3.cutfrom(shape)
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self.mainpanel = HeatSinkMain(self.name+"_mainpanel",self.origin)
        finCenterorig = self.origin.add(Base.Vector(self._finXOffset,\
                                                    self._finCenterYOffset,0))
        fin1orig = finCenterorig.add(Base.Vector(0,\
                                                 -HeatSinkFin._lengths[1]/2.0,\
                                                 -125))
        self.fin1 = HeatSinkFin(self.name+"_fin1",fin1orig,1)
        fin2orig = finCenterorig.add(Base.Vector(0,\
                                                 -HeatSinkFin._lengths[2]/2.0,\
                                                 -250))
        self.fin2 = HeatSinkFin(self.name+"_fin2",fin2orig,2)
        fin3orig = finCenterorig.add(Base.Vector(0,\
                                                 -HeatSinkFin._lengths[3]/2.0,\
                                                 -375))
        self.fin3 = HeatSinkFin(self.name+"_fin3",fin3orig,3)
