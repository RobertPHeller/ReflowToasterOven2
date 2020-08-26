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
#  Created       : Tue Aug 25 11:05:39 2020
#  Last Modified : <200826.0926>
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
    
class ACPowerEntry(object):
    _flangewidthmax = 1970
    _flangeheightmax = 880
    _flangeholes = { 0 : Base.Vector(0,197.5,440), \
                     1 : Base.Vector(0,1772.5,440) }
    _flangeholeradius = 138.0/2.0
    _cutoutYoff = (1970-1080)/2.0
    _cutoutZoff = (880-790)/2.0
    _cutoutwidth = 1080
    _cutoutheight = 790
    _flangeXoff = 80
    _flangeXdepth = 120
    _totaldepthX = 1120
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name)
        obj.Shape = self.body
        obj.Label=self.name
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
    def MountingHole(self,index,depth=None,xPanelOff=0):
        if depth == None:
            depth = self._flangeXdepth
        p = self._flangeholes[index]
        if self.end == 'back':
            depth *= -1
        if self.orientation == 'horizontal':
            holePos = self.origin.add(Base.Vector(xPanelOff,p.y,p.z))
        else:
            holePos = self.origin.add(Base.Vector(xPanelOff,p.z,p.y))
        return Part.Face(Part.Wire(Part.makeCircle(self._flangeholeradius,\
                                                   holePos,\
                                                   Base.Vector(1,0,0)))\
                        ).extrude(Base.Vector(depth,0,0))
    def __init__(self,name,origin,end='front',orientation='horizontal'):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        if end not in ['front', 'back']:
            raise RuntimeError("end is not front or back!")
        self.end = end
        if orientation not in ['horizontal', 'vertical']:
            raise RuntimeError("orientation is not horizontal or vertical!")
        self.orientation = orientation
        if self.end == 'front':
            flangeXoff = self._flangeXoff
            totaldepthX = self._totaldepthX
            flangeXdepth = self._flangeXdepth
            norm = Base.Vector(1,0,0)
        elif self.end == 'back':
            flangeXoff = -1 * self._flangeXoff
            totaldepthX = -1 * self._totaldepthX
            flangeXdepth = -1 * self._flangeXdepth
            norm = Base.Vector(-1,0,0)
        else:
            pass
        if self.orientation == 'horizontal':
            cutoutYoff = self._cutoutYoff
            cutoutZoff = self._cutoutZoff
            cutoutwidth = self._cutoutwidth
            cutoutheight = self._cutoutheight
            flangewidthmax = self._flangewidthmax
            flangeheightmax = self._flangeheightmax
        elif self.orientation == 'vertical':
            cutoutYoff = self._cutoutZoff
            cutoutZoff = self._cutoutYoff
            cutoutwidth = self._cutoutheight
            cutoutheight = self._cutoutwidth
            flangewidthmax = self._flangeheightmax
            flangeheightmax = self._flangewidthmax
        else:
            pass
        origBody = self.origin.add(Base.Vector(flangeXoff,cutoutYoff,\
                                                cutoutZoff))
        origBody = origBody.add(Base.Vector(0,norm.x*cutoutwidth,0))
        self.cutoutSurf = Part.makePlane(cutoutheight,cutoutwidth,origBody,\
                                         norm)
        self.body = self.cutoutSurf.extrude(Base.Vector(totaldepthX,0,0))
        flangeOrig = self.origin.add(Base.Vector(0,norm.x*flangewidthmax,0))
        flange = Part.makePlane(flangeheightmax,flangewidthmax,flangeOrig,norm\
                               ).extrude(Base.Vector(flangeXdepth,0,0))
        self.body = self.body.fuse(flange)
        self.body = self.body.cut(self.MountingHole(0))
        self.body = self.body.cut(self.MountingHole(1))
    def Cutout(self):
        if self.end == 'front':
            totaldepthX = self._totaldepthX
        elif self.end == 'back':
            totaldepthX = -1 * self._totaldepthX
        else:
            pass
        return self.cutoutSurf.extrude(Base.Vector(totaldepthX,0,0))

class ACReceptacle(object):
    _totalwidth = MM2Mils(27)
    _totalheight = MM2Mils(27)
    _totaldepth = MM2Mils(32.77)
    _bodywidth = MM2Mils(26)
    _bodyheight = MM2Mils(22)
    _bodyYoff = MM2Mils((27-26)/2.0)
    _bodyZoff = MM2Mils((27-22)/2.0)
    _flangeXoffset = -MM2Mils(3.6)
    def _holepolygon(self,xPanel):
        cutoutWidth = MM2Mils(26)
        cutoutHeight = MM2Mils(22)
        lowerTabWidth = MM2Mils(19)
        upperTabWidth = MM2Mils(9.1)
        middleTabHeight = MM2Mils(10.6)
        lowerTabHeight = MM2Mils(5.75)
        upperTabHeight = MM2Mils(22 - (10.6 + 5.75))
        holepolygon = list()
        dy1 = (ACReceptacle._totalwidth-lowerTabWidth)/2.0
        dz1 = (ACReceptacle._totalheight-cutoutHeight)/2.0
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy1,dz1)))
        dz2 = dz1 + lowerTabHeight
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy1,dz2)))
        dy2 = (ACReceptacle._totalwidth-cutoutWidth) / 2.0
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy2,dz2)))
        dz3 = dz2 + middleTabHeight
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy2,dz3)))
        dy3 = (ACReceptacle._totalwidth-upperTabWidth) / 2.0
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy3,dz3)))
        dz4 = dz3 + upperTabHeight
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy3,dz4)))
        dy4 = dy3 + upperTabWidth
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy4,dz4)))
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy4,dz3)))
        dy5 = dy2 + cutoutWidth
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy5,dz3)))
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy5,dz2)))
        dy6 = dy1 + lowerTabWidth
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy6,dz2)))
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy6,dz1)))
        holepolygon.append(self.origin.add(Base.Vector(xPanel,dy1,dz1)))
        return holepolygon
    def MountHole(self,depth=0,xPanel=0):
        mpoly = self._holepolygon(xPanel)
        if self.end == 'back':
            depth *= -1
        return Part.Face(Part.Wire(Part.makePolygon(mpoly))\
                        ).extrude(Base.Vector(depth,0,0))
    def show(self):
        doc = App.activeDocument()
        obj = doc.addObject("Part::Feature",self.name)
        obj.Shape = self.body
        obj.Label=self.name
        obj.ViewObject.ShapeColor=tuple([0.0,0.0,0.0])
    def __init__(self,name,origin,end='front'):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        if end not in ['front', 'back']:
            raise RuntimeError("end is not front or back!")
        self.end = end
        if self.end == 'front':
            flangeXoffset = -1 * self._flangeXoffset
            totaldepth = self._totaldepth
            norm = Base.Vector(1,0,0)
        elif self.end == 'back':
            flangeXoffset = self._flangeXoffset
            totaldepth = -1 * self._totaldepth
            norm = Base.Vector(-1,0,0)
        else:
            pass
        bodyOrig = self.origin.add(Base.Vector(0,\
                                (-norm.x*self._bodywidth)+(-norm.x*self._bodyYoff),\
                                (-norm.x*self._bodyheight)+(-norm.x*self._bodyZoff)))
        self.body = Part.makePlane(self._bodyheight,self._bodywidth,\
             bodyOrig,norm).extrude(Base.Vector(totaldepth-flangeXoffset,0,0))
        flangeOrig = self.origin.add(Base.Vector(-flangeXoffset,-(norm.x*self._totalwidth),-norm.x*self._totalheight))
        flange = Part.makePlane(self._totalwidth,self._totalheight,flangeOrig,\
                                norm).extrude(Base.Vector(flangeXoffset,0,0))
        self.body = self.body.fuse(flange)


if __name__ == '__main__':
    App.ActiveDocument=App.newDocument("AC")
    doc = App.activeDocument()
    backrecept = ACReceptacle("AC",Base.Vector(0,0,0),end='back')
    backrecept.show()
    panel = Part.makePlane(3000,3000,Base.Vector(0,1500,1500),Base.Vector(-1,0,0)).extrude(Base.Vector(-MM2Mils(2)))
    panel = panel.cut(backrecept.MountHole(MM2Mils(2)))
    obj = doc.addObject("Part::Feature","Panel")
    obj.Shape = panel
    obj.ViewObject.ShapeColor=tuple([0.0,0.0,1.0])
    origin = Part.Wire(Part.makeCircle(backrecept._bodyYoff,Base.Vector(0,0,0),Base.Vector(-1,0,0))).extrude(Base.Vector(100,0,0))
    obj = doc.addObject("Part::Feature","origin")
    obj.Shape = origin
    obj.ViewObject.ShapeColor=tuple([1.0,1.0,0.0])
    Gui.SendMsgToActiveView("ViewFit") 
    Gui.activeDocument().activeView().viewIsometric()
