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
                    zbar_scanner_new_scan(scn){ // 返回是哪种类型的码
                        zbar_scanner_flush(){
                            process_edge(){
                                zbar_decode_width()//返还码的类型,通过边缘的宽度
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
                        qr_finder_centers_locate(){
                            // 要内存
                            //对过定位符的一组线排序形成簇
                            // 找定位符的中心点s
                            }
                        if(ncenters >= 3){
                            // 找到定位符后再二值化
                            qr_binarize();
                            qr_code_data_list_init(&qrlist);
                            qr_reader_match_centers(){ // QR图像处理的主要部分 
                                // 极其重要的
                                // 遍历所有的定位符中心,选其中三个来处理
                                qr_reader_try_configuration(){
                                    // 利用叉乘判断三个点的关系，共线就退出
                                    // affine-transformation 准备
                                    // 将三个定位符中心做
                                    qr_aff_unproject();
                                    // 定位符边缘像素分类??
                                    qr_finder_edge_pts_aff_classify(&ur,&aff);
                                    // 定位符估计模块大小(方形区域)和版本
                                    qr_finder_estimate_module_size_and_version();
                                    // 遇到不合适的就进一步检测下一个可能的的定位符

                                    // 如果比较合理就做full homography
                                    // 这里面有和timming patern有关了
                                    qr_hom_fit(){
                                        // 拟合QR码的四条边缘线
                                        // l[0] l[2] 很准确, l[1] l[３]要麻烦些
                                    }
                                    qr_hom_unproject();
                                }
                            }

                            qr_code_data_list_extract_text(){
                                // 译码
                            }
                            
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