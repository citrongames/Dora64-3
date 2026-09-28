"""Synthetic checks for wrong-section dispatch and invalid overlay declarations. No ROM/game run."""
import copy,struct,unittest,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from overlay_layout import prepare_layout

class OverlayLayoutCheck(unittest.TestCase):
    def setUp(self):
        self.rom=bytearray(0x70000)
        self.meta={'compiled':{},'sdk':{},'overlays':[
            {'name':'.a','rom':0x60000,'vram':0x80100000,'size':0x100,'overlay_group':'scene','compiled':{'0x80100000':['a_entry',8,'synthetic']},'indirect_targets':{}},
            {'name':'.b','rom':0x60100,'vram':0x80100000,'size':0x100,'overlay_group':'scene','compiled':{'0x80100000':['b_entry',8,'synthetic'],'0x80100020':['b_target',8,'synthetic']},'indirect_targets':{}}]}
        struct.pack_into('>I',self.rom,0x60000,0x0C000000|((0x80100020>>2)&0x3ffffff))
        struct.pack_into('>I',self.rom,0x60100,0x0C000000|((0x80100020>>2)&0x3ffffff))
    def test_same_vram_does_not_borrow_other_overlay_function(self):
        _,compiled,_,pending=prepare_layout(self.meta,self.rom)
        self.assertEqual(compiled[1,0x80100000][0],'a_entry')
        self.assertEqual(compiled[2,0x80100000][0],'b_entry')
        self.assertEqual(pending,{(1,0x80100020)})
    def test_unmarked_overlap_rejected(self):
        del self.meta['overlays'][1]['overlay_group']
        with self.assertRaisesRegex(ValueError,'RAM overlap'):prepare_layout(self.meta,self.rom)
    def test_rom_overlap_rejected(self):
        self.meta['overlays'][1]['rom']=0x60000
        with self.assertRaisesRegex(ValueError,'ROM sections'):prepare_layout(self.meta,self.rom)
    def test_external_ambiguous_call_rejected(self):
        self.meta['compiled']['0x80025C00']=['entry',8,'synthetic']
        struct.pack_into('>I',self.rom,0x1000,0x0C000000|((0x80100000>>2)&0x3ffffff))
        with self.assertRaisesRegex(ValueError,'Ambiguous external call'):prepare_layout(self.meta,self.rom)
    def test_duplicate_native_symbol_rejected(self):
        self.meta['overlays'][1]['compiled']['0x80100000'][0]='a_entry'
        with self.assertRaisesRegex(ValueError,'Invalid function'):prepare_layout(self.meta,self.rom)

    def add_child(self):
        self.meta['overlays'].append({'name':'.child','rom':0x60200,'vram':0x80200000,'size':0x100,
            'requires':['.a'],'compiled':{'0x80200000':['child',8,'synthetic']},'indirect_targets':{}})
        struct.pack_into('>I',self.rom,0x60200,0x0C000000|((0x80100020>>2)&0x3ffffff))
    def test_required_parent_does_not_borrow_menu_target(self):
        self.add_child()
        _,_,_,pending=prepare_layout(self.meta,self.rom)
        self.assertEqual(pending,{(1,0x80100020)})
    def test_unknown_parent_rejected(self):
        self.add_child();self.meta['overlays'][-1]['requires']=['.missing']
        with self.assertRaisesRegex(ValueError,'Invalid required'):prepare_layout(self.meta,self.rom)
    def test_conflicting_parents_rejected(self):
        self.add_child();self.meta['overlays'][-1]['requires']=['.a','.b']
        with self.assertRaisesRegex(ValueError,'coexist'):prepare_layout(self.meta,self.rom)
    def test_dependency_cycle_rejected(self):
        self.add_child();self.meta['overlays'][0]['requires']=['.child']
        with self.assertRaisesRegex(ValueError,'Cyclic'):prepare_layout(self.meta,self.rom)

if __name__=='__main__':unittest.main()
