AudioSignalResClass {
 Inputs {
  IOPItemInputClass {
   id 4
   name "M1"
   tl -306 -275.072
   children {
    6
   }
  }
  IOPItemInputClass {
   id 5
   name "M2"
   tl -304 -128
   children {
    6
   }
  }
  IOPItemInputClass {
   id 7
   name "Input 7"
   tl -304 -16
   children {
    6
   }
  }
  IOPItemInputClass {
   id 8
   name "Input 8"
   tl -288 80
   children {
    6
   }
  }
 }
 Ops {
  IOPItemOpMulClass {
   id 6
   name "Mul BVS"
   tl -63 -211.643
   children {
    3
   }
   inputs {
    ConnectionClass "7:0" {
     id 7
     port 0
    }
    ConnectionClass "5:0" {
     id 5
     port 0
    }
    ConnectionClass "8:0" {
     id 8
     port 0
    }
    ConnectionClass "4:0" {
     id 4
     port 0
    }
   }
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 3
   name "MultiplyBS"
   tl 160 -208
   input 6
  }
 }
 Input_Order {
  ItemDetailListItemClass M1 {
   Name "M1"
   Id 4
  }
  ItemDetailListItemClass M2 {
   Name "M2"
   Id 5
  }
  ItemDetailListItemClass "Input 7" {
   Name "Input 7"
   Id 7
  }
  ItemDetailListItemClass "Input 8" {
   Name "Input 8"
   Id 8
  }
 }
 Output_Order {
  ItemDetailListItemClass MultiplyBS {
   Name "MultiplyBS"
   Id 3
  }
 }
}