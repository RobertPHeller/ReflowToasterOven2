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
#  Created       : Mon Aug 24 14:21:00 2020
#  Last Modified : <200824.1422>
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
    
from ham_1591XXTSBK import ham_1591XXTSBK

class UpperBox(ham_1591XXTSBK):
    @property
    def upsidedown(self):
        return False
    @property
    def includelid(self):
        return True
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self._buildbox()
        
class LowerBox(ham_1591XXTSBK):
    @property
    def upsidedown(self):
        return True
    @property
    def includelid(self):
        return False
    def __init__(self,name,origin):
        self.name = name
        if not isinstance(origin,Base.Vector):
            raise RuntimeError("origin is not a Vector!")
        self.origin = origin
        self._buildbox()



if __name__ == '__main__':
    if "ham_1591XXTSBK" in App.listDocuments().keys():
        App.closeDocument("ham_1591XXTSBK")
    App.ActiveDocument=App.newDocument("ham_1591XXTSBK")
    doc = App.activeDocument()
    upperbox = UpperBox("upper",Base.Vector(0,0,0))
    upperbox.show()
    lowerbox = LowerBox("lower",Base.Vector(0,0,0))
    lowerbox.show()
    Gui.SendMsgToActiveView("ViewFit")
    Gui.activeDocument().activeView().viewIsometric()

