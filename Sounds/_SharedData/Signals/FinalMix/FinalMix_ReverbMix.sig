AudioSignalResClass {
 Inputs {
  IOPInputValueClass {
   id 3
   name "Small Start [m3]"
   tl -256 -624
   children {
    5 10
   }
   value 100
  }
  IOPInputValueClass {
   id 4
   name "Medium Start [m3]"
   tl -256 -432
   children {
    9 10
   }
   value 3500
  }
  IOPInputValueClass {
   id 6
   name "Medium End [m3]"
   tl -256 -352
   children {
    9 10
   }
   value 18000
  }
  IOPInputValueClass {
   id 7
   name "Small End [m3]"
   tl -256 -544
   children {
    5 10
   }
   value 1500
  }
  IOPInputValueClass {
   id 39
   name "Exponent"
   tl 256 -848
   children {
    32 33 34
   }
   value 0.5
  }
  IOPInputVariableClass {
   id 41
   name "GRoomSize"
   tl -454.174 -769.611
   ctl 0 -24
   children {
    40
   }
   varName "GRoomSize"
   varResource "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf"
  }
 }
 Ops {
  IOPItemOpInterpolateClass {
   id 5
   name "Interpolate Small"
   tl 0 -768
   children {
    32
   }
   inputs {
    ConnectionClass connection {
     id 3
     port 1
    }
    ConnectionClass connection {
     id 40
     port 0
    }
    ConnectionClass connection {
     id 7
     port 2
    }
   }
   "Y min" 1
   "Y max" 0
  }
  IOPItemOpInterpolateClass {
   id 9
   name "Interpolate Large"
   tl 0 -256
   children {
    34
   }
   inputs {
    ConnectionClass connection {
     id 4
     port 1
    }
    ConnectionClass connection {
     id 40
     port 0
    }
    ConnectionClass connection {
     id 6
     port 2
    }
   }
  }
  IOPItemOpEnvClass {
   id 10
   name "Interpolate Medium"
   tl 0 -512
   children {
    33
   }
   inputs {
    ConnectionClass connection {
     id 7
     port 2
    }
    ConnectionClass connection {
     id 6
     port 4
    }
    ConnectionClass connection {
     id 3
     port 1
    }
    ConnectionClass connection {
     id 40
     port 0
    }
    ConnectionClass connection {
     id 4
     port 3
    }
   }
  }
  IOPItemOpPowClass {
   id 32
   name "Pow Small"
   tl 256 -768
   children {
    11
   }
   inputs {
    ConnectionClass connection {
     id 39
     port 1
    }
    ConnectionClass connection {
     id 5
     port 0
    }
   }
  }
  IOPItemOpPowClass {
   id 33
   name "Pow Medium"
   tl 256 -512
   children {
    12
   }
   inputs {
    ConnectionClass connection {
     id 39
     port 1
    }
    ConnectionClass connection {
     id 10
     port 0
    }
   }
  }
  IOPItemOpPowClass {
   id 34
   name "Pow Large"
   tl 256 -256
   children {
    13
   }
   inputs {
    ConnectionClass connection {
     id 39
     port 1
    }
    ConnectionClass connection {
     id 9
     port 0
    }
   }
  }
  IOPItemOpConvertorClass {
   id 40
   name "Converter "
   tl -256 -768
   children {
    5 9 10
   }
   inputs {
    ConnectionClass connection {
     id 41
     port 0
    }
   }
   DefaultFromInput 1
   Intervals {
    IOPItemOpConvertorRange Default {
     out 2500
    }
   }
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 11
   name "Small_V"
   tl 512 -768
   input 32
  }
  IOPItemOutputClass {
   id 12
   name "Medium_V"
   tl 512 -512
   input 33
  }
  IOPItemOutputClass {
   id 13
   name "Large_V"
   tl 512 -256
   input 34
  }
 }
 compiled IOPCompiledClass {
  visited {
   645 775 517 389 261 133 135 647 262 5 263 519 134 7 391 6
  }
  ins {
   IOPCompiledIn {
    data {
     2 3 131075
    }
   }
   IOPCompiledIn {
    data {
     2 65539 131075
    }
   }
   IOPCompiledIn {
    data {
     2 65539 131075
    }
   }
   IOPCompiledIn {
    data {
     2 3 131075
    }
   }
   IOPCompiledIn {
    data {
     3 196611 262147 327683
    }
   }
   IOPCompiledIn {
    data {
     1 393219
    }
   }
  }
  ops {
   IOPCompiledOp {
    data {
     1 196611 6 0 1 393217 0 196608 2
    }
   }
   IOPCompiledOp {
    data {
     1 327683 6 65536 1 393217 0 131072 2
    }
   }
   IOPCompiledOp {
    data {
     1 262147 10 196608 2 131072 4 0 1 393217 0 65536 3
    }
   }
   IOPCompiledOp {
    data {
     1 2 4 262144 1 1 0
    }
   }
   IOPCompiledOp {
    data {
     1 65538 4 262144 1 131073 0
    }
   }
   IOPCompiledOp {
    data {
     1 131074 4 262144 1 65537 0
    }
   }
   IOPCompiledOp {
    data {
     3 3 65539 131075 2 327680 0
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
  }
  processed 16
  version 2
  ins_reeval_list {
   5
  }
 }
}