AudioSignalResClass {
 Inputs {
  IOPItemInputClass {
   id 1
   name "DynamicRange"
   tl -413.266 160.306
   children {
    4 37 50 55
   }
   value 1
   valueMin -1
  }
 }
 Ops {
  IOPItemOpInterpolateClass {
   id 4
   name "Interpolate Loud"
   tl -72 -23.25
   children {
    49
   }
   inputs {
    ConnectionClass connection {
     id 1
     port 0
    }
   }
   EnableCustomCurve 1
   CustomCurve {
    CurvePoint "1" {
     X -1
     Y -3
    }
    CurvePoint "2" {
    }
    CurvePoint "3" {
     X 1
     Y 1
    }
   }
  }
  IOPItemOpInterpolateClass {
   id 37
   name "Interpolate Mid"
   tl -73.532 148.552
   children {
    38
   }
   inputs {
    ConnectionClass connection {
     id 1
     port 0
    }
   }
   EnableCustomCurve 1
   CustomCurve {
    CurvePoint "1" {
     X -1
     Y 3
    }
    CurvePoint "2" {
    }
    CurvePoint "3" {
     X 1
    }
   }
  }
  SignalOpDb2GainClass {
   id 38
   name "Db2Gain 19"
   tl 175.143 153.579
   children {
    39
   }
   inputs {
    ConnectionClass connection {
     id 37
     port 0
    }
   }
  }
  SignalOpDb2GainClass {
   id 49
   name "Db2Gain 16"
   tl 175.857 -23.762
   children {
    2
   }
   inputs {
    ConnectionClass connection {
     id 4
     port 0
    }
   }
  }
  IOPItemOpInterpolateClass {
   id 50
   name "Interpolate Quiet"
   tl -73.254 322.539
   children {
    54
   }
   inputs {
    ConnectionClass connection {
     id 1
     port 0
    }
   }
   EnableCustomCurve 1
   CustomCurve {
    CurvePoint "1" {
     X -1
     Y 3
    }
    CurvePoint "2" {
    }
    CurvePoint "3" {
     X 1
     Y -1
    }
   }
  }
  SignalOpDb2GainClass {
   id 54
   name "Db2Gain 19"
   tl 175.143 326.25
   children {
    51
   }
   inputs {
    ConnectionClass connection {
     id 50
     port 0
    }
   }
  }
  IOPItemOpInterpolateClass {
   id 55
   name "Interpolate Quiet"
   tl -65 517.5
   children {
    56
   }
   inputs {
    ConnectionClass connection {
     id 1
     port 0
    }
   }
   EnableCustomCurve 1
   CustomCurve {
    CurvePoint "1" {
     X -1
     Y 4
    }
    CurvePoint "2" {
     Y 2
    }
    CurvePoint "3" {
     X 1
     Y 1.5
    }
   }
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 2
   name "Loud_V"
   tl 411.69 -24.762
   input 49
  }
  IOPItemOutputClass {
   id 39
   name "Medium_V"
   tl 410.976 156.345
   input 38
  }
  IOPItemOutputClass {
   id 51
   name "Quiet_V"
   tl 410.976 325.635
   input 54
  }
  IOPItemOutputClass {
   id 56
   name "Compressor_Ratio"
   tl 412.5 521.25
   input 55
  }
 }
 compiled IOPCompiledClass {
  visited {
   5 775 390 519 647 262 135 263 134 7 391 6
  }
  ins {
   IOPCompiledIn {
    data {
     4 3 65539 262147 393219
    }
   }
  }
  ops {
   IOPCompiledOp {
    data {
     1 196611 2 0 0
    }
   }
   IOPCompiledOp {
    data {
     1 131075 2 0 0
    }
   }
   IOPCompiledOp {
    data {
     1 65538 2 65537 0
    }
   }
   IOPCompiledOp {
    data {
     1 2 2 1 0
    }
   }
   IOPCompiledOp {
    data {
     1 327683 2 0 0
    }
   }
   IOPCompiledOp {
    data {
     1 131074 2 262145 0
    }
   }
   IOPCompiledOp {
    data {
     1 196610 2 0 0
    }
   }
  }
  outs {
   IOPCompiledOut {
    data {
     0
    }
   }
   IOPCompiledOut {
    data {
     0
    }
   }
   IOPCompiledOut {
    data {
     0
    }
   }
   IOPCompiledOut {
    data {
     0
    }
   }
  }
  processed 12
  version 2
 }
}