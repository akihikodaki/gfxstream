
# Autogenerated makefile
# ../protocols/vk-gen/xml/genvk.py -registry ../protocols/vk-gen/xml/vk.xml cereal -o ../host/vulkan/cereal
# Please do not modify directly;
# re-run generate-vulkan-sources.sh,
# or directly from Python by defining:
# VULKAN_REGISTRY_XML_DIR : Directory containing genvk.py and vk.xml
# CEREAL_OUTPUT_DIR: Where to put the generated sources.
# python3 $VULKAN_REGISTRY_XML_DIR/genvk.py -registry $VULKAN_REGISTRY_XML_DIR/vk.xml cereal -o $CEREAL_OUTPUT_DIR

android_add_big_protobuf(OpenglRender_vulkan_cereal_proto )

