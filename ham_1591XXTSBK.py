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
#  Created       : Mon Aug 24 13:34:12 2020
#  Last Modified : <200826.1119>
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
    
class ham_1591XXTSBK(object):
    __metaclass__ = ABCMeta
    _topouterwidth = MM2Mils(83.27)
    _topouterlength = MM2Mils(123.27)
    _bottomouterwidth = MM2Mils(75.65 + 2 + 2)
    _bottomouterlength = MM2Mils(115.65 + 2 + 2)
    _outsideheight = MM2Mils(56.25)
    _insidewidth = MM2Mils(75.65)
    _insidelength = MM2Mils(115.65)
    _insideheight = MM2Mils(55.00)
    _wallthickness = MM2Mils(2.00)
    @property
    @abstractmethod
    def upsidedown(self):
        return False
    @property
    @abstractmethod
    def includelid(self):
        return True
    @property
    def InsideFrontX(self):
        return self.origin.x+self._wallthickness
    @property
    def InsideFrontY(self):
        return self.origin.y+self._wallthickness
    @property
    def OutsideFrontX(self):
        return self.origin.x - \
                    ((self._topouterlength-self._bottomouterlength)/2.0)
    @property
    def OutsideFrontY(self):
        return self.origin.y - \
                    ((self._topouterwidth-self._bottomouterwidth)/2.0)
    @property
    def InsideBottomZ(self):
        if self.upsidedown:
            return self.origin.z - self._wallthickness
        else:
            return self.origin.z + self._wallthickness
    @property
    def TopZ(self):
        if self.upsidedown:
            return self.origin.z - self._outsideheight
        else:
            return self.origin.z + self._outsideheight
    @property
    def InsideWidth(self):
        return self._insidewidth
    @property
    def InsideLength(self):
        return self._insidelength
    @property
    def InsideHeight(self):
        return self._insideheight
    @property
    def OutsideWidth(self):
        return self._topouterwidth
    @property
    def OutsideLength(self):
        return self._topouterlength
    @property
    def BottomOuterWidth(self):
        return self._bottomouterwidth
    @property
    def BottomOuterLength(self):
        return self._bottomouterlength
    @property
    def WallThickness(self):
        if self.upsidedown:
            return -1*self._wallthickness
        else:
            return self._wallthickness
    _lidholes = { 1 : Base.Vector(310.4330708661417,310.4330708661417,0), \
                  2 : Base.Vector(4542.716535433071,310.4330708661417,0), \
                  3 : Base.Vector(4542.716535433071,2967.9133858267714,0), \
                  4 : Base.Vector(310.4330708661417,2967.9133858267714,0) }
    def LidHole(self,i,Z,thick):
        lhorig = Base.Vector(self.lidscrew.x,self.lidscrew.y,Z\
                            ).add(self._lidholes[i])
        return Part.Face(Part.Wire(Part.makeCircle(62.5,lhorig))).extrude(Base.Vector(0,0,thick))
    def _buildbox(self):
        xoff = self.origin.x
        yoff = self.origin.y
        zoff = self.origin.z
        lowerBackX = xoff+self._bottomouterlength
        upperBackX = xoff+(self._bottomouterlength+\
                        ((self._topouterlength-self._bottomouterlength)/2.0))
        lowerFrontX = xoff
        upperFrontX = xoff - ((self._topouterlength-self._bottomouterlength)/2.0)
        upperLeftY = yoff - ((self._topouterwidth-self._bottomouterwidth)/2.0)
        lowerLeftY = yoff
        upperRightY = yoff + (self._bottomouterwidth+((self._topouterwidth-self._bottomouterwidth)/2.0))
        lowerRightY = yoff + self._bottomouterwidth
        lowerZ = zoff
        if self.upsidedown:
            upperZ = zoff - self._outsideheight
            wallthicknessZ = -1 * self._wallthickness
            screwZ = upperZ - self._wallthickness
        else:
            upperZ = zoff + self._outsideheight
            wallthicknessZ = self._wallthickness
            screwZ = upperZ + self._wallthickness
        bottom = Part.makePlane(self._bottomouterlength,\
                                self._bottomouterwidth,\
                                Base.Vector(lowerFrontX,lowerLeftY,lowerZ)\
                               ).extrude(Base.Vector(0,0,wallthicknessZ))
        leftPoly = [Base.Vector(lowerFrontX,lowerLeftY,lowerZ), \
                    Base.Vector(lowerBackX,lowerLeftY,lowerZ), \
                    Base.Vector(upperBackX,upperLeftY,upperZ), \
                    Base.Vector(upperFrontX,upperLeftY,upperZ), \
                    Base.Vector(lowerFrontX,lowerLeftY,lowerZ)]
        left = Part.Face(Part.Wire(Part.makePolygon(leftPoly))).extrude(Base.Vector(0,self._wallthickness,0))
        rightPoly = [Base.Vector(lowerFrontX,lowerRightY,lowerZ), \
                     Base.Vector(lowerBackX,lowerRightY,lowerZ), \
                     Base.Vector(upperBackX,upperRightY,upperZ), \
                     Base.Vector(upperFrontX,upperRightY,upperZ), \
                     Base.Vector(lowerFrontX,lowerRightY,lowerZ)]
        right = Part.Face(Part.Wire(Part.makePolygon(rightPoly))).extrude(Base.Vector(0,-self._wallthickness,0))
        frontpoly = [Base.Vector(lowerFrontX,lowerLeftY,lowerZ),\
                     Base.Vector(upperFrontX,upperLeftY,upperZ),\
                     Base.Vector(upperFrontX,upperRightY,upperZ),\
                     Base.Vector(lowerFrontX,lowerRightY,lowerZ),\
                     Base.Vector(lowerFrontX,lowerLeftY,lowerZ)]
        front = Part.Face(Part.Wire(Part.makePolygon(frontpoly))).extrude(Base.Vector(self._wallthickness,0,0))
        backpoly = [Base.Vector(lowerBackX,lowerLeftY,lowerZ),\
                    Base.Vector(upperBackX,upperLeftY,upperZ),\
                    Base.Vector(upperBackX,upperRightY,upperZ),\
                    Base.Vector(lowerBackX,lowerRightY,lowerZ),\
                    Base.Vector(lowerBackX,lowerLeftY,lowerZ)]
        back = Part.Face(Part.Wire(Part.makePolygon(backpoly))).extrude(Base.Vector(-self._wallthickness,0,0))
        self.box = bottom.fuse(left).fuse(right).fuse(front).fuse(back)
        self.lid = None
        self.lidscrew  = Base.Vector(upperFrontX,upperLeftY,screwZ)
        if self.includelid:
            self.lidorigin = Base.Vector(upperFrontX,upperLeftY,upperZ)
            lidthick  = Base.Vector(0,0,wallthicknessZ)
            self.lid = Part.makePlane(self._topouterlength,self._topouterwidth,
                                      self.lidorigin).extrude(lidthick)
            for lh in [1,2,3,4]:
                self.lid = self.lid.cut(self.LidHole(lh,screwZ,-wallthicknessZ))
    def cutbox(self,obj):
        self.box = self.box.cut(obj)
    def cutlid(self,obj):
        if self.lid != None:
            self.lid = self.lid.cut(obj)
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name)
        obj.Shape = self.box
        obj.Label=self.name
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
        if self.lid != None:
            obj = doc.addObject("Part::Feature",self.name+"_lid")
            obj.Shape = self.lid
            obj.Label=self.name+"_lid"
            obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])

