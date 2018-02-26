main()
{
    // 命令行获取图片
    zbar_processor_create(){
        // 获取一片内存区域
    }
    scan_image(){
        // 加载图片，得到大小等操作
        zbar_process_image(){
            _zbar_process_image(){
                
                zbar_scan_image(){
                    _zbar_qr_reset() 
                    // 图片格式转化，得到灰度图
                    // 回收上一个scanner 图片结果
                    zbar_scanner_new_scan(scn){ // 返回边缘
                        zbar_scanner_flush(){
                            process_edge(){
                                zbar_decode_width()//返还码的类型?
                            }

                        }

                        zbar_decoder_new_scan(scn->decoder){
                            qr_finder_reset(&dcode->qrf);
                        }
                    }
                    zbar_scan_y(){

                    }
                    // 真正的QR解码
                    _zbar_qr_decode(){
                        svg_group_start("finder", 0, 1. / (1 << QR_FINDER_SUBPREC), 0, 0, 0){

                        }
                        qr_finder_centers_locate(){
                            // 找定位符的中心点s
                        }
                        if(ncenters >= 3){
                            // 找到定位符后再二值化
                            qr_binarize();
                            qr_code_data_list_init(&qrlist);

                            qr_reader_match_centers(){ // QR图像处理的主要部分 
                                // 极其重要的

                            }

                            qr_code_data_list_extract_text(){
                                // 译码
                            }
                            
                        }
                    }
                    // *****
                }
            }
        }
        // 输出解码信息
    }
}